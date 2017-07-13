import os
import re
import random
import time

f = open("allfp.txt", "r")
test_data = 45
ROUND_NUM = 30
r = random.randint(0,200)
simi = 0.0


def similarity(a,b):
    k = 0
    for i in range(len(b)):
        if(a[i]==b[i]):
            k+=1
    return k/len(b)

for line_number, line in enumerate(f,0):
    if(line_number < r):
        continue

    if(line_number >= test_data+r):
        break
    s, original_fp = line.split(':')
    cmd = "./attack {}".format(s)

    # s = sys.argv[1]
    length = 80*2
    res = [[0 for p in range(16)] for q in range(1024)]
    indexArray = [str(x).zfill(2) for x in range(16)]
    ans = [[0 for p in range(4)] for q in range(1024)]
    time.sleep(3)

    for i in range(random.randint(ROUND_NUM - 5, ROUND_NUM + 5)):
        t = os.popen(cmd).readlines()

        f = open("cache_traces.txt", 'r')
        j = 0
        for line in f:
            a = re.compile(":?\s+").split(line)
            z = 0
            if(a[0] == ''):
                z = 1
            for p in range(16):
                k = int(a[p+z])
                if(k>90):
                    k = 70
                res[j][p] = k
            m = min(res[i][0:4])
            m_index = 0
            for k in range(1,4):
                minRes = min(res[j][k*4:(k+1)*4])
                if(minRes > m):
                    m_index = k
                    m = minRes
                elif(minRes == m):
                    if(sum(res[j][k*4:(k+1)*4]) > sum(res[j][m_index*4:(m_index+1)*4])):
                        m_index = k
            ans[j][m_index] += 1
            j+=1
        f.close()

    # for i in range(1024):
    #     for j in range(16):
    #         res[i][j] /= ROUND_NUM
    # for i in range(0,length,2):
    #     print(ans[i])

    fingerPrint = ""
    for i in range(length):
        if(i%2 == 0):
            m = ans[i][0]
            m_index = 0
            for k in range(1,4):
                minRes = ans[i][k]
                if(minRes > m):
                    m_index = k
                    m = minRes
            # print(m_index)
            # print(" ".join(str(x) for x in indexArray))
            # print(" ".join(str(int(x)) for x in res[i]))

            fingerPrint += str(m_index)

    print("original fp: ",original_fp)
    print("very new fp: ",fingerPrint[:60])
    k = similarity(original_fp, fingerPrint[:len(original_fp)])
    simi += k
    print(s, k)
        # print s, i
        # f.write("###########%s#############\n" % i)
        # f.writelines(t)
# cPickle.dump(res, open("res.db","w"))
print("average similarity: ",simi/test_data)


