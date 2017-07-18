import os
import re
import random
import time

f = open("fps.txt", "r")
test_data = 200
ROUND_NUM = 5
r = 0
simi = 0.0


def similarity(a,b):
    k = 0
    j = min(len(a), len(b))
    for i in range(j):
        if(a[i]==b[i]):
            k+=1
    return k/j

for line_number, line in enumerate(f,0):
    if(line_number < r):
        continue

    if(line_number >= test_data+r):
        break
    s, original_fp = line.split(' ')
    cmd = "taskset -c 2 ./reference {}".format(s)

    # s = sys.argv[1]
    length = 80
    res = [[[0 for p in range(64)] for q in range(1024)] for x in range(ROUND_NUM)]
    indexArray = [str(x).zfill(2) for x in range(64)]
    ans = [[0 for p in range(4)] for q in range(1024)]
    probable_fp = ""
    for i in range(ROUND_NUM):
        t = os.popen(cmd).readlines()

        for j, line in enumerate(t,0):
            a = re.compile(":?\s+").split(line)
            z = 0
            if(a[0] == ''):
                z = 1
            for p in range(64):
                k = int(a[p+z])
                if(k>70):
                    k = 70
                res[i][j][p] = k


    for i in range(64):
        k = 0
        for j in range(ROUND_NUM):
            for p in range(300):
                k += res[j][p][i]
        k //= (ROUND_NUM*300)
        for j in range(ROUND_NUM):
            for p in range(300):
                res[j][p][i] -= k


    for i in range(ROUND_NUM):
        for j in range(300):
            m = min(res[i][j][0:16])
            m_index = 0
            for k in range(1,4):
                minRes = min(res[i][j][k*16:(k+1)*16])
                if(minRes > m):
                    m_index = k
                    m = minRes
                elif(minRes == m):
                    if(sum(res[i][j][k*16:(k+1)*16]) > sum(res[i][j][m_index*16:(m_index+1)*16])):
                        m_index = k
            ans[j][m_index] += 1

    # for i in range(1024):
    #     for j in range(16):
    #         res[i][j] /= ROUND_NUM
    # for i in range(0,length,2):
    #     print(ans[i])

    fingerPrint = ""
    complete_fp = ""
    for i in range(length):
        m = ans[i][0]
        m_index = 0
        for k in range(1,4):
            minRes = ans[i][k]
            if(minRes > m):
                m_index = k
                m = minRes
    #     # print(m_index)
    #     # print(" ".join(str(x) for x in indexArray))
    #     # print(" ".join(str(int(x)) for x in res[i]))
        # if(i%2 == 0):
        complete_fp += str(m_index)
    #         fingerPrint += str(m_index)

    # print("original fp: ",original_fp)
    # print("complete fp: ",complete_fp)
    # print("very new fp: ",fingerPrint[:60])
    k = similarity(original_fp, complete_fp)
    print(s,k)
    # simi += k
    # print(s, k)
        # print s, i
        # f.write("###########%s#############\n" % i)
        # f.writelines(t)
# cPickle.dump(res, open("res.db","w"))
# print("average similarity: ",simi/test_data)


