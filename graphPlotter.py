import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt

f = open("temp.txt", 'r')
cx = []
cy = []
sx = []
px = []
for line in f:
    if line[0] == 'c':
        _, a, b = line.split(" ")
        a = int(a)
        b = int(b)
        if(b>40):
            b = 2
        else:
            b = 1
        cx.append(a)
        cy.append(b)

    elif line[0] == 's':
        _, a, b= line.split(" ")
        sx.append(int(a))

    elif line[0] == 'p':
        _, a, b = line.split(" ")
        px.append(int(a))

    else:
        continue

sy = [1 for i in range(len(sx))]
py = [1 for i in range(len(px))]

print(len(px))
print(len(sx))
# print(len(cx))
# plt.plot(cx[:800],cy[:800], 'r.')
# plt.plot(sx[:200],sy[:200], 'g.')
for l in sx[:200]:
    plt.axvline(l)
plt.plot(px[:200],py[:200], 'b.')
# plt.axis([cx[0], cx[800], 0, 2])
fig = plt.gcf()
fig.set_size_inches(37, 10, forward = True)
fig.savefig('s.png', dpi=400)

# plt.plot(x[:100],y[:100], 'r.')
# plt.figure(figsize=(20,10))
# plt.savefig("s.png")
