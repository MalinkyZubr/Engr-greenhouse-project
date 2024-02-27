y = [2.00]


for x in range(1,10000):
    y.append(2.0 + (9.0 / y[x-1]))
    
    
print(y[-1])