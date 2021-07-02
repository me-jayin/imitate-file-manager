#include<stdlib.h>

#include<iostream>
using namespace std;

#define UNIT_BIT 8
#define BIT_MAP_SIZE 512
#define BIT_TOTAL_BIT 4096

class BitEmptyRange {
	public:
		int min; // 占用位开始下标 
		int max; // 占用位结束下标 
		BitEmptyRange() {
			min = max = -1;
		}
		BitEmptyRange(int mi, int ma) {
			min = mi;
			max = ma;
		}
};

class BitMap {
	private:
		char map[BIT_MAP_SIZE];
		/** 使用的bit数 */
		int count;
	public:
		BitMap() {
			// 初始化字节数组 
			for (int i = 0; i <= BIT_MAP_SIZE; i++) {
				map[i] = 0;
			}
			count = 0;
		};
		~BitMap() {
			free(map);
		} 
		
		int getMaxSize() {
			return BIT_MAP_SIZE;
		}
		
		/**
		 * 得到空闲的空间 
		 */ 
		int getFreeCount() {
			return BIT_MAP_SIZE - count;
		}
		
		/**
		 * 设置位图位置，其中offset从0开始， status为1表示被使用，为0表示未使用 
		 */
		bool set(int offset, int status) {
			if (offset < 0 || offset >= BIT_TOTAL_BIT) return false;
			
			int idx = offset / UNIT_BIT, bitOffset = offset % UNIT_BIT;
			char target = map[idx];
			// 判断初始状态和将要设置的是否一样 
			if (!((target >> bitOffset) ^ status)) return false; 
			map[idx] = target ^ (1 << bitOffset);
			
			if (status) count++;
			else count--;
			return true;
		}
		
		/**
		 * 获取某一位的状态 
		 */
		bool get(int offset) {
			if (offset < 0 || offset > BIT_TOTAL_BIT) return false;
			
			int idx = offset / UNIT_BIT, bitOffset = offset % UNIT_BIT;
			char target = map[idx];
			return (target >> bitOffset) & 1;
		}
		
		/**
		 * 给指定范围的数据设置状态 
		 */
		bool setRange(int start, int end, int status) {
			// 解析校验设置范围内的状态是否已经和目标状态一样 
			for (int i = start; i <= end; i++) {
				if (this->get(i) == status) return false;
			}
			 
			for (int i = start; i <= end; i++) {
				this->set(i, status);
			}
			return true; 
		}
		
		/**
		 * 设置指定范围的值 
		 */
		bool setRange(BitEmptyRange *range, int status) {
			this->setRange(range->min, range->max, status);
		}
		
		/**
		 * 获取为空的数据块范围 
		 */
		BitEmptyRange* getEmptyRange() {
			return this->getEmptyRange(BIT_MAP_SIZE);
		}
		
		/**
		 * 获取指定大小范围为空的数据库范围 
		 */
		BitEmptyRange* getEmptyRange(int size) {
			return getEmptyRange(0, size);
		}
		
		/**
		 * 获取指定位置后未使用的位范围
		 */
		BitEmptyRange* getEmptyRange(int pos, int size) {
			int min = -1, max = -1;
			for (int i = pos; i < BIT_MAP_SIZE && size; i++) {
				if (!this->get(i)) {
					if (min == -1) min = i;
					max = i;
					size--;
				} else if (min != -1) {
					break;
				}
			}
			return new BitEmptyRange(min, max);
		}
		
		/**
		 * 获取第一个未被使用的bit 
		 */
		int getFristEmpty() {
			for (int i = 0; i < BIT_MAP_SIZE; i++) {
				if (!this->get(i)) {
					return i;
				}
			}
			return -1;
		}
        
        /**
         * 获取从指定位置（包括指定位置）已被使用的bit 
         */
        int getFirstUse(int pos) {
        	for (int i = pos; i < BIT_MAP_SIZE; i++) {
        		if (this->get(i)) {
					return i;
				}
			}
			return -1;
		}
		
		void display() {
			for (int i = 0; i < BIT_TOTAL_BIT; i++) {
				if (i % 32 == 0 && i != 0) cout << endl;
				cout << this->get(i) << " ";
			}
			cout << endl;
		}
				
		char* getMap() {
			return map;
		}
};
