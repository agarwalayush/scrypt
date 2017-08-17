import os
import re
import random
import string

f = open("temp", "r")
test_data = 200
ROUND_NUM = 15
r = 0
simi = 0.0
sets = 16
block = sets//4
length = 120

def gen_rnd_str(N=16):
    return ''.join(random.choice(string.ascii_uppercase + string.digits + string.ascii_lowercase) for _ in range(N))

def similarity(a, b):
    k = 0
    j = min(len(a), len(b))
    for i in range(j):
        if(a[i] == b[i]):
            k += 1
    return k/j


# for line_number, line in enumerate(f, 0):
for line_number, l in enumerate(f, 0):
    if(line_number%2 == 1):
        continue

    line = l.split(' ')
    s = line[0]
    original_fp = line[2]
    cmd = "taskset -c 2 ./attack {}".format(s)

    # s = sys.argv[1]
    res = [[[0 for p in range(64)] for q in range(1024)]
           for x in range(ROUND_NUM)]

    ans = [[0 for p in range(4)] for q in range(1024)]
    probable_fp = ""
    for i in range(ROUND_NUM):
        os.popen(cmd).readlines()
        t = open("cache_traces.txt")

        for j, line in enumerate(t, 0):
            a = re.compile(":?\s+").split(line)
            z = 0
            if(a[0] == ''):
                z = 1
            for p in range(sets):
                k = int(a[p+z])
                if(k > 70):
                    k = 70
                res[i][j][p] = k

        t.close()

    for i in range(sets):
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
            m = min(res[i][j][0:block])
            m_index = 0
            for k in range(1, 4):
                minRes = min(res[i][j][k*block:(k+1)*block])
                if(minRes > m):
                    m_index = k
                    m = minRes
                elif(minRes == m):
                    if(sum(res[i][j][k*block:(k+1)*block]) >
                       sum(res[i][j][m_index*block:(m_index+1)*block])):
                        m_index = k
            ans[j][m_index] += 1

    fingerPrint = ""
    complete_fp = ""
    for i in range(length):
        m = ans[i][0]
        m_index = 0
        for k in range(1, 4):
            minRes = ans[i][k]
            if(minRes > m):
                m_index = k
                m = minRes
    #     # print(" ".join(str(x) for x in indexArray))
    #     # print(" ".join(str(int(x)) for x in res[i]))
        # if(i%2 == 0):
        complete_fp += str(m_index)
    #         fingerPrint += str(m_index)

    # print("original fp: ",original_fp)
    # print("complete fp: ",complete_fp)
    k = similarity(original_fp[:30], complete_fp[:30])
    print(s, complete_fp[:30], original_fp[:30], k)
