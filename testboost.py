import libMakeData
import numpy as np
import time
import matplotlib.pyplot as plt

def timeit(function):
    def wrapper(*args, **kwargs):
        tb = time.time()
        result = function(*args, **kwargs)
        te = time.time()
        print(te - tb)
        return result
    return wrapper


dm = libMakeData.DataMaker()

dat = np.transpose(dm.GenerateDataMatrix(10000))
names = dm.SpectrumNames
for name in names:
    print(name)
##print(dat)
##dm.PrintData()
#A = timeit(dm.test)()
#B = timeit(np.array)(A)
#C = timeit(dm.convert_to_numpy)(A)
print(np.shape(dat))

x = np.array([x for x in range(0,4096)],dtype=int)
y = dat[:,0]
print(np.shape(y))

plt.plot(x,y,'b')
plt.show()

print(np.sum(y))
