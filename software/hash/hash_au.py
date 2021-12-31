import numpy as np
import math


class set_associate_hash_table:
    def __init__(self, table_size=8192, ways=2, H=180, W=240):
        self.H = H
        self.W = W
        self.table_size = table_size // ways
        self.in_width = math.ceil(math.log2(H * W))
        self.table_width = math.ceil(math.log2(self.table_size))
        self.a = 72189
        self.A_key = np.zeros([self.table_size, ways], dtype=np.uint32)
        self.A_value = np.zeros([self.table_size, ways])
        self.collision_count = 0
        self.dependency_count = 0

    def hash_map(self, x, y):
        return y * self.W + x + 1
    
    def hash_forward(self, x, a):
        temp = int(x * a) >> (self.in_width - self.table_width)
        temp = temp & (2**(self.table_width) - 1)
        return int(temp)


    def interested(self, x, y):
        key, idx_a, key_a, val_a = self.query(x, y)
        if ((key in key_a) and (val_a[np.where(key_a == key)] > 0)):
            return True
        else:
            return False

    def query(self, x, y):
        val_a = 0
        key = self.hash_map(x, y)
        idx_a = self.hash_forward(key, self.a)
        key_a = self.A_key[idx_a]
        val_a = self.A_value[idx_a]
        #print("key:", key, "idx_a:", idx_a, "key_a:", key_a, "val_a:", val_a, "idx_b:", idx_b, "key_b:", key_b, "val_b:", val_b)
        return key, idx_a, key_a, val_a
    
    def add(self, x, y):
        key, idx_a, key_a, val_a = self.query(x, y)
        #print(key, idx_a, key_a, val_a)
        if(key in key_a):
            index = np.where(key_a == key)
            self.A_value[idx_a, index] += 1

        #elif(0 in key_a or 0 in val_a):
        elif(0 in val_a):
            next_zero_index = np.where(val_a == 0)[0][0]
            self.A_key[idx_a, next_zero_index] = key
            self.A_value[idx_a, next_zero_index] = 1

        else:
            self.collision_count += 1
            min_index = np.argmin(val_a)
            self.A_key[idx_a, min_index] = key
            self.A_value[idx_a, min_index] = 1

    
    def sub(self, x, y):
        key, idx_a, key_a, val_a = self.query(x, y)
        if(key in key_a):
            index = np.where(key_a == key)
            if (val_a[index] > 0):
                self.A_value[idx_a, index] -= 1
            
    def convert_to_dense(self):
        idxs = np.where(self.A_key!= 0)
        keys = self.A_key[idxs]
        x = (keys - 1) % self.W
        y = (keys - 1) // self.W
        val = self.A_value[idxs]
        self.hmap = np.zeros([self.H, self.W], dtype=np.uint8)
        self.hmap[y, x] = val
        return self.hmap
        
        
    def test_dependency_rate(self):
        self.idx = np.zeros([self.H, self.W], dtype=np.uint32)
        for y in range(self.H - 1):
            for x in range(self.W - 1):
                self.idx[y, x] = self.hash_forward(self.hash_map(x, y), self.a)
        return self.idx
            
  