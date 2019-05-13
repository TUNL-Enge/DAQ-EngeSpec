import libMakeData
import numpy as np
import time


def timeit(function):
    def wrapper(*args, **kwargs):
        tb = time.time()
        result = function(*args, **kwargs)
        te = time.time()
        print(te - tb)
        return result
    return wrapper


dm = libMakeData.DataMaker()

dm.GenerateDataMatrix(1000)

dm.PrintData()
#A = timeit(dm.test)()
#B = timeit(np.array)(A)
#C = timeit(dm.convert_to_numpy)(A)
