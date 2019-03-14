import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

n1 = 1000000
cov = [[200**2, 0],[0,200**2]]
mean = [510, 1200]
x1, y1 = np.random.multivariate_normal(mean,cov,n1).T

n2 = 10000000
cov = [[500**2,300**2],[300**2,500**2]]
mean = [2400, 1700]
x2, y2 = np.random.multivariate_normal(mean,cov,n2).T

## Put the two together and then shuffle them
x = np.concatenate((x1,x2))
y = np.concatenate((y1,y2))
arr = np.arange(n1+n2)
np.random.shuffle(arr)
x = x[arr]
y = y[arr]

## Make a pandas data frame and save
df = pd.DataFrame(np.array([x,y]).T, columns=['Pos1','DE'])
df.to_hdf('2Ddata-large.hdf', key='df', mode='w')
df.to_pickle("2Ddata-large.pkl")

plt.hist2d(x, y,bins=[256,256],range=[[0,4096],[0,4096]])
#plt.axis('equal')
plt.show()

