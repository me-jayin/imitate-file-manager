#include<stdio.h>
#include<string.h>

/**
 * 判断字符串str是否是prefix开头 
 */
bool strStartWith(const char* str, const char* prefix) {
	if (str == prefix) return false;
	
	int i = 0;
	while (str[i] != '\0' && prefix[i] != '\0') {
		if (str[i] != prefix[i]) return false;
		i++;
	}
	// str字符串已经完了 
	if (str[i] == '\0' && prefix[i] != '\0') return false;
	return true;
}




