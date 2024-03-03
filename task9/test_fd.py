import os

fd0 = os.open("/dev/solution_node", os.O_RDWR | os.O_NOCTTY)
f0 = open(fd0, "wb+", buffering=0)
f1 = open(fd0, "wb+", buffering=0)

print("Session 0 Testing that first read returns valid sid...", f0.readline())
print("Session 1 Testing that first read returns valid sid...", f1.read())



print("Session 0 seek(0,2): 0[]", f0.seek(0,2))
print("Session 1 seek(1,0): 2[]", f1.seek(1,0))
