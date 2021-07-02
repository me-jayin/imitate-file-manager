using namespace std;

/** 一个位表示的空间大小 */
#define BIT_BLKSIZE 32 
const char* FILE_TABLE = "filetable.dat";

class FileManager {
    public:
	    DirectoryTable *table; // 文件项表 
	    BitMap *bitMap; // 位示图 
		DirectoryItem *cdi; // 当前所在目录 
		DirectoryItem *ofile; // 当前打开的文件 
	    FileManager() {
	    	table = new DirectoryTable();
	    	bitMap = new BitMap();
	    	cdi = NULL;
	    	ofile = NULL;
		}
		const char* getCurrentDirectoryName() {
			return this->cdi == NULL ? "/" : this->cdi->filename;
		}
		void displayChildDirectory(User *user);
		void createDirectory(User *user, string filename);
		void createFile(User *user, string filename);
		DirectoryItem* findByInum(short inum); 
		void cd(User *user, string filename);
		void rm(User *user, string target);
		void openFile(User *user, string filename);
	private:
		DirectoryItem* _userFileItem(User *user, string filename, int type);
		int getInum() {
	        if (this->cdi == NULL) return -1;
	        return this->cdi->inum;
        }
        void fileOperate();
		void read();
		void write(); 
		void releaseOpenFileBit(int size); 
		void mallocOpenFileBit(int size, bool retry);
		void saveFile(string content);
		void rm(User *user, DirectoryItem *item, bool isTop);
};

void saveFmData(FileManager *fm);
void saveFmData(FileManager *fm, ofstream *outFile);
void freeDirectoryArray(DirectoryArray *ary); 

/**
 * 列出所有子文件夹 
 */
void FileManager::displayChildDirectory(User *user) {
	int inum = getInum();
	
	// 获取所有子文件 
	DirectoryArray *ary = table->findByParent(user->usr, inum);

	// 计算子文件总数 
	int count = ary == NULL ? 0 : ary->count;
	// 子文件中文件夹数量 及 文件大小 
	int dircount = 0, fileTotal = 0;
	// 遍历所有子文件 
	for (int i = 0; i < count; i++) {
		DirectoryItem *item = ary->items[i];
		cout << item->filename << "\t\t";
		// 按类型输出 
		if (item->type == DIRECTORY) {
			dircount++;
			cout << "<DIR>";
		} else {
			cout << "<FILE>\t\t";
			int ft = item->getUseBitCount() * BIT_BLKSIZE;
			cout << " total size " << ft << " bytes, actaul size " << item->length << " bytes ";
			fileTotal += ft;
		}
		cout << endl;
	}
	
	// 输出总览 
	cout << "\t\t" << (count - dircount) << " file(s) " << fileTotal << " bytes " << endl;
	cout << "\t\t" << dircount << " dir(s) " << (bitMap->getFreeCount() * BIT_BLKSIZE) << " bytes, free space " << endl;
    freeDirectoryArray(ary);
}

/**
 * 使用用户文件先内置方法 
 */ 
DirectoryItem* FileManager::_userFileItem(User *user, string filename, int type) {
	int inum = getInum();
	
	// 判断文件是否存在 
	string tf = (type == DIRECTORY ? "文件夹" : "文件");
	if (table->fileExistBy(user->usr, inum, filename)) {
		cout << "创建失败，" << tf << "已存在" << endl;
		return NULL;
	}
	
	// 获取可用空项 
	DirectoryItem *empty = table->useEmptyItem();
	if (empty == NULL) {
		cout << tf << "创建失败，FAT表容量已满" << endl;
		return NULL;
	}
	// 创建文件夹 
	empty->iparent = inum;
	strcpy(empty->filename, filename.c_str());
	empty->type = type;
	strcpy(empty->usr, user->usr);
	return empty;
}

/** 
 * 创建文件夹 
 */
void FileManager::createDirectory(User *user, string filename) {
	DirectoryItem *empty = this->_userFileItem(user, filename, DIRECTORY);
	if (empty == NULL) return;
	
	cout << "文件夹创建完成" << endl;
	// 实时写入到文件中 
	saveFmData(this);
}

void FileManager::createFile(User *user, string filename) {
	DirectoryItem *empty = this->_userFileItem(user, filename, NORMAL_FILE);
	if (empty == NULL) return;
	
	empty->length = 0;
	cout << "文件创建完成" << endl;
	// 实时写入到文件中 
	saveFmData(this);
}

DirectoryItem* FileManager::findByInum(short inum) {
	return table->find(inum);
}

void FileManager::cd(User *user, string filename) {
	// 如果是 cd .. 或 cd ../ 则到上一级目录 
	if (filename.compare("..") == 0 || filename.compare("../") == 0) {
		if (this->cdi == NULL) {
			cout << "已经无父级目录" << endl;
		    return;
		} else { // 得到父级目录 
			short pinum = this->cdi->iparent;
			// 如果无父级则直接到根目录 
			if (pinum == -1) this->cdi = NULL;
			else {
				this->cdi = this->findByInum(pinum);
			}
		    return;
		}
	} else if (filename.compare("/") == 0) {
		this->cdi = NULL;
		return;
	}
	
	int inum = getInum();
	// 获取所有子文件 
	DirectoryArray *ary = table->findByParent(user->usr, inum);
	if (ary == NULL) {
		cout << "操作失败，文件夹不存在" << endl;
		return;
	}
	
	for (int i = 0; i < ary->count; i++) {
		// 找到同名文件 
		if (strcmp(filename.c_str(), ary->items[i]->filename) == 0) {
			DirectoryItem *temp = ary->items[i];
		    freeDirectoryArray(ary);
		    // 判断类型 
			if (temp->type != DIRECTORY) {
				cout << "操作失败，目标非文件夹" << endl;
				return;
			}
			this->cdi = temp;
		    return;
		}
	}
	cout << "操作失败，文件夹不存在" << endl;
}

void FileManager::rm(User *user, string filename) {
	int inum = getInum();
	// 获取所有子文件 
	DirectoryArray *ary = table->findByParent(user->usr, inum);
	if (ary == NULL) {
		cout << "操作失败，文件不存在" << endl;
		return;
	}
	
	for (int i = 0; i < ary->count; i++) {
		// 找到同名文件 
		if (strcmp(filename.c_str(), ary->items[i]->filename) == 0) {
			DirectoryItem *temp = ary->items[i];
		    freeDirectoryArray(ary);
		    rm(user, temp, true);
			// 保存文件管理对象数据 
			saveFmData(this);
		    return;
		}
	}
	
	cout << "操作失败，文件不存在" << endl;
}

/**
 * 删除文件，如果是文件夹先提示，然后递归删除，如果是文件直接删除并释放bitmap 
 */
void FileManager::rm(User *user, DirectoryItem *item, bool isTop) {
	if (item->type == DIRECTORY) { // 如果是文件夹则遍历子文件并删除自身 
		if (isTop) { // 如果是在顶层文件是文件夹就提示 
			cout << "是否删除文件夹 [" << item->filename << "] 及其里面的内容？(Y/N)" << endl;
			cout << "> ";
			string temp;
			getline(cin, temp);;
			if (temp.compare("Y") != 0 && temp.compare("y") != 0) {
				return;
			}
		}
		// 获取所有子文件 
		DirectoryArray *ary = table->findByParent(user->usr, item->inum);
		if (ary != NULL) {
			for (int i = 0; i < ary->count; i++) {
				rm(user, ary->items[i], false);
			}
		}
		item->release();
	} else if (item->type == NORMAL_FILE) { // 如果是文件直接删除并释放bitmap 
		// 释放位示图 
		for (int i = 0; i < BIT_RANGE_SIZE; i++) {
			if (item->address[i].min == -1) continue;
			this->bitMap->setRange(&item->address[i], 0);
			item->address[i].min = item->address[i].max = -1;
		}
		item->release();
	}
}

void FileManager::openFile(User *user, string filename) {
	int inum = getInum();
	// 获取所有子文件 
	DirectoryArray *ary = table->findByParent(user->usr, inum);
	if (ary == NULL) {
		cout << "操作失败，文件夹不存在" << endl;
		return;
	}
	
	for (int i = 0; i < ary->count; i++) {
		// 找到同名文件 
		if (strcmp(filename.c_str(), ary->items[i]->filename) == 0) {
			DirectoryItem *temp = ary->items[i];
		    freeDirectoryArray(ary);
		    // 判断类型 
			if (temp->type != NORMAL_FILE) {
				cout << "操作失败，目标非可打开文件" << endl;
				return;
			}
			this->ofile = temp;
			// 输出打开文件后的命令
			this->fileOperate();
		    return;
		}
	}
    cout << "操作失败，指定文件不存在" << endl;
} 

void FileManager::fileOperate() {
	string cmd;
	while (true) {
		cout << "----------------------------" << endl;
		cout << "请输入需要对文件 [" << this->ofile->filename << "] 执行的操作：" << endl;
		cout << "    read    读取文件内容" << endl;
		cout << "    write   写入文件内容" << endl;
		cout << "    close   关闭文件" << endl;
		cout << "> ";
		cin >> cmd;
		if (strcmp(cmd.c_str(), "read") == 0) {
			read();
		} else if (strcmp(cmd.c_str(), "write") == 0) {
		    write(); 
		} else if (strcmp(cmd.c_str(), "close") == 0) {
			cout << "文件关闭成功" << endl;
		    cout << "----------------------------" << endl;
		    getchar();
			return;
		} else {
			cout << "无效的命令" << endl;
		}
	}
}

/**
 * 释放当前打开文件占用的位视图占位 
 */
void FileManager::releaseOpenFileBit(int size) {
	int bitc = 0, releaseMin, releaseMax;
	// 从后往前释放 
	for (int i = BIT_RANGE_SIZE - 1; i >= 0 && size > 0; i--) {
		BitEmptyRange *rg = &this->ofile->address[i];
		if (rg->min == -1) continue;
		// 得到当前范围的bit数 
		bitc = rg->max - rg->min + 1;
		
		if (bitc > size) { // 如果当前位数位置范围含有的位数大于需要释放的位数 
			releaseMax = rg->max;
			rg->max -= size; // 进行释放指定size位后的max值 
			releaseMin = rg->max + 1;
		} else { // 如需要释放的位数大于这一范围的位数，则直接释放这一范围并位数减去当前大小继续执行 
			releaseMin = rg->min;
			releaseMax = rg->max;
			size -= bitc;
			rg->min = rg->max = -1;
		}
		this->bitMap->setRange(releaseMin, releaseMax, 0);
	}
} 

/**
 * 为当前文件分配指定大小的位示图空间 
 */
void FileManager::mallocOpenFileBit(int size, bool retry) {
	BitEmptyRange *address = this->ofile->address;
	int rangeSize = 0;
	// 先统计当前已经使用的范围数
	for (; rangeSize < BIT_MAP_SIZE; rangeSize++) {
		BitEmptyRange rg = address[rangeSize];
		if (rg.min < 0) break;
	}
	
	// 开始分配范围
	for (; rangeSize < BIT_RANGE_SIZE && size > 0; rangeSize++) {
		BitEmptyRange* range = this->bitMap->getEmptyRange(size);
		size -= (range->max - range->min + 1);
		// 分配bit位 
		this->bitMap->setRange(range->min, range->max, 1);
		// 添加范围
		address[rangeSize].min = range->min;
		address[rangeSize].max = range->max;
		
		free(range);
	}
	// 判断分配的大小是否足够 
	if (size > 0 && rangeSize == BIT_MAP_SIZE && !retry) { // 如果大于size大于0，并且分配位数组已满 
		this->ofile->adjustBitRange();
		// 磁盘碎片整理
		// ----------------- 
		mallocOpenFileBit(size, true);
		return; 
	}
}

void FileManager::read() {
	int length = this->ofile->length;
	if (!length) return;
	ifstream inFile(FILE_TABLE, ios::in | ios::binary);
	
	cout << "-------------- " << this->ofile->filename << " --------------" << endl;
	for (int i = 0; i < BIT_RANGE_SIZE; i++) {
		BitEmptyRange *range = &this->ofile->address[i];
		if (range->min == -1) continue;
		// 将范围内的位对应的块逐个读数据 
		char temp[BIT_BLKSIZE];
		for (int i = range->min; i <= range->max; i++) {
			inFile.seekg(sizeof(char) * BIT_BLKSIZE * i, ios::beg);
			inFile.read((char*) temp, sizeof(temp));
			
			// 将读出来的数据打印 
			for (int j = 0; j < BIT_BLKSIZE && j < length; j++) {
				cout << temp[j];
			}
			
			// 计算剩余字符数 
			length -= BIT_BLKSIZE; 
		}
	}
	cout << endl;
	cout << endl;
	inFile.close();
}

void FileManager::write() {
	// 得到文件当前占用的位数量，以及总共可用的位数 
	int ouBitc = this->ofile->getUseBitCount(), 
	        freeBitc = ouBitc + this->bitMap->getFreeCount();
	cout << "----------------------------" << endl;
	cout << "当前磁盘可用大小（包括当前文件占用）   " << (freeBitc * BIT_BLKSIZE) << " bytes" << endl;
	cout << "        :q   退出编辑" << endl;
	cout << "        :w   保存编辑" << endl; 
	cout << "请输入文件内容：" << endl;
	
	string input, temp = "";
	while (1) {
		cin >> input;
	    if (strcmp(input.c_str(), ":q") == 0) {
		    return;
	    } else if (strcmp(input.c_str(), ":w") == 0) {
	    	break;
		}
		if (temp.compare("") != 0) {
			temp.append("\n");
		}
		temp.append(input);
	}
	
    // 计算当前编辑后所需要的占用的位数
    int strlen = temp.length(), needBitc = (strlen / BIT_BLKSIZE) + (strlen % BIT_BLKSIZE == 0 ? 0 : 1);
    BitEmptyRange *range = this->ofile->address;
    if (needBitc > freeBitc) { // 超出剩余内存 
    	cout << "文件超出磁盘可用大小，编辑失败" << endl;
    	return;
	} else if (needBitc <= ouBitc) { // 小于原来的占用大小，需要是否部分位；等于的话，可以直接覆盖原来的数据   
		int releaseCount = ouBitc - needBitc; // 得到需要释放的
	    this->releaseOpenFileBit(releaseCount);
	} else if (needBitc > ouBitc) { // 大于原来的数据的话，需要新增占用的部分位
	    int mallocCount = needBitc - ouBitc; // 需要分配的数量 
	    this->mallocOpenFileBit(mallocCount, false);
    }
//	int nbitc = 
	this->ofile->length = strlen; 
	saveFile(temp);
	cout << ">>> 文件编辑成功" << endl;
}

void FileManager::saveFile(string content) {
	ofstream outFile(FILE_TABLE, ios::out | ios::binary| ios::in); //二进制写方式打开
	
	int strOffset = 0;
	BitEmptyRange *address = this->ofile->address;
	// 开始试分配范围
	for (int i = 0; i < BIT_RANGE_SIZE; i++) {
		BitEmptyRange *range = &address[i];
		if (range->min == -1) continue;
		// 将范围内的位对应的块逐个输出到 dat 
		for (int i = range->min; i <= range->max; i++) {
			// 按块切割字符串 
			const char *sub = content.substr(strOffset++ * BIT_BLKSIZE, BIT_BLKSIZE).c_str();
			// 将字符输入位置指到位块对应的位置 
			outFile.seekp(sizeof(char) * BIT_BLKSIZE * i, ios::beg);
			// 将字符串输入 
			outFile.write(sub, sizeof(char) * BIT_BLKSIZE);
		}
	}
	// 保存文件管理对象数据 
	saveFmData(this, &outFile);
	outFile.close();
}

//---------------
// 全局函数 
//---------------
/**
 * 释放文件夹数组对象 
 */
void freeDirectoryArray(DirectoryArray *ary) {
	if (ary == NULL) return;
	free(ary->items);
	free(ary);	
}
/**
 * 保存文件管理器数据，使用指定的文件输出方式保存 
 */ 
void saveFmData(FileManager *fm, ofstream *outFile) {
	outFile->seekp(sizeof(char) * BIT_BLKSIZE * BIT_MAP_SIZE - 1, ios::beg);
	// 输出table和位图信息
	outFile->write((char*) fm->table, sizeof(DirectoryTable));
	outFile->write((char*) fm->bitMap, sizeof(BitMap));
}

/**
 * 保存文件管理器数据，使用默认读写方式保存 
 */
void saveFmData(FileManager *fm) {
	ofstream outFile(FILE_TABLE, ios::out | ios::binary| ios::in); //二进制写方式打开
	saveFmData(fm, &outFile);
	outFile.close();
}

/**
 * 初始化文件管理数据 
 */
void initFmDat(FileManager *fm) {
	ofstream outFile(FILE_TABLE, ios::trunc | ios::binary); //二进制写方式打开
	// 先填充指定硬盘数据 
	cout << "数据准备中..." << endl;
	char c[1];
	c[0] = '\0';
	for (int i = 0; i < BIT_BLKSIZE * BIT_TOTAL_BIT; i++) {
		outFile.write((char*) c, sizeof(c));
	}
	saveFmData(fm, &outFile);
	outFile.close();
} 

/**
 * 进行文件系统初始化，init为是否初始化 
 */
FileManager* initFileManager(bool init) {
	FileManager *fm = new FileManager();
	
	ifstream fmd(FILE_TABLE, ios::in | ios::binary);
	if (!fmd.good() || init) {
		cout << "是否进行系统文件初始化(Y/N)？";
		string temp;
		cin >> temp;
	    getchar();
		if (temp.compare("Y") == 0 || temp.compare("y") == 0) {
			fmd.close();
			// 初始化文件 
			initFmDat(fm);
			cout << "系统初始化完成" << endl;
			return fm;
		} else { // 退出 
			return NULL;
		}
	}
	
	// 读取数据
	fmd.seekg(sizeof(char) * BIT_BLKSIZE * BIT_TOTAL_BIT - 1, ios::beg);
	// 输出table和位图信息
	fmd.read((char*) fm->table, sizeof(DirectoryTable));
	fmd.read((char*) fm->bitMap, sizeof(BitMap));
	fmd.close();
	return fm;
}

