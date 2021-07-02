
/** 文件类型：路径 */ 
#define DIRECTORY 1
/** 文件类型：普通文件 */
#define NORMAL_FILE 2

// 共128个table项 
#define TABLE_ITEM_SIZE 128
// bit范围数组大小 
#define BIT_RANGE_SIZE 5

/**
 * 目录项 
 */
class DirectoryItem {
	public:
		short inum; // 文件i节点号 
		char filename[10]; // 文件名
		char type; // 文件类型
		char usr[10]; // 文件所有者
		short iparent; // 父目录的i节点号  
		short length; // 文件长度
		BitEmptyRange address[BIT_RANGE_SIZE]; // 存放文件的地址    
		DirectoryItem() {
			this->release();
		}
		void release() {
			inum = -1;
			iparent = -1;
			length = 0;
			type = -1;
		}
		/**
		 * 得到当前文件使用的位数 
		 */
		int getUseBitCount() {
			int count = 0, i = 0;
			for (; i < BIT_RANGE_SIZE; i++) {
				BitEmptyRange rg = address[i];
				if (rg.min == -1) continue;
				count += (rg.max - rg.min) + 1;
			}
			return count;
		}
		
		void adjustBitRange() {
			int offset = 1;
			for (int i = 0, last; i + offset < BIT_RANGE_SIZE;) {
				last = i + offset;
				
				if (address[i].min == -1) { // 如果当前范围为空，则把下一个非空范围挪到前面，并偏移到下一个 
					address[i].min = address[last].min;
					address[i].max = address[last].max;
					address[last].min = address[last].max = -1;
					offset++; 
				} else if (address[last].min == -1) {
					offset++;
				} else if (address[i].max == address[last].min - 1) { // 如果后面的最小和当前的最大是相邻的则进行合并 
					address[i].max = address[last].max;
					address[last].min = address[last].max = -1;
					offset++; // 对比位置偏移一个 
				} else { // 如果当前和偏移的不成相邻，那么当前往下挪，偏移位不变 
					i++;
					if (offset > 1) offset--;
				}
			} 
		}
};

class DirectoryArray {
	public:
		int count; // 符合要求的文件项个数 
		DirectoryItem **items; // 文件项集 
		DirectoryArray(int count, DirectoryItem **items) {
			this->count = count;
			this->items = items;
		}
};

/**
 * 文件表 
 */
class DirectoryTable {
	public:
		DirectoryItem items[TABLE_ITEM_SIZE];
		DirectoryTable() {}
		
		/**
		 * 找到指定节点 
		 */
		DirectoryItem* find(short inum) {
			for (int i = 0; i < TABLE_ITEM_SIZE; i++) {
				if (items[i].inum == inum) {
					return &items[i];
				}
			}
			return NULL;
		}
		
		bool fileExistBy(char *usr, short inum, string filename) {
			// 遍历得到所有子文件数组坐标 
			for (int i = 0; i < TABLE_ITEM_SIZE; i++) {
				// 当前用户，并且为当前目录下的同名文件 
 				if (items[i].inum > -1 && strcmp(usr, items[i].usr) == 0 
				 		&& items[i].iparent == inum && strcmp(items[i].filename, filename.c_str()) == 0) {
					return true;
				}
			}
			return false;
		}
		
		/**
		 * 找到指定节点子节点 
		 */
		DirectoryArray* findByParent(char *usr, short inum) {
			int idx[TABLE_ITEM_SIZE] = {-1}, count = 0;
			// 遍历得到所有子文件数组坐标 
			for (int i = 0; i < TABLE_ITEM_SIZE; i++) {
 				if (items[i].inum > -1 && strcmp(usr, items[i].usr) == 0 && items[i].iparent == inum) {
					idx[count++] = i;
				}
			}
			if (!count) return NULL;
			
//			DirectoryItem *_items[count];
			DirectoryItem **_items = (DirectoryItem**) malloc(sizeof(DirectoryItem *) * count);
			for (int i = 0; i < count; i++) {
				_items[i] = &items[idx[i]];
			}
			return new DirectoryArray(count, _items);
		}
		
		/**
		 * 找到空节点 
		 */
		DirectoryItem* useEmptyItem() {
			for (int i = 0; i < TABLE_ITEM_SIZE; i++) {
				if (items[i].inum == -1) {
					items[i].inum = i;
					return &items[i];
				}
			}
			return NULL;
		}
		
		/** 
		 * 释放节点 
		 */
		void freeItem(short inum) {
			DirectoryItem *item = find(inum);
			if (item != NULL) item->release();
		}
		
		
};


