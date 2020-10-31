#include <lzUtils/base.h>
#include <getopt.h>
#include <string>
#include <map>
#include <future>
#include <iostream>
#include <vector>
using namespace std;
 
void map1(){
 
    map<int, string> mapStudent;
	pair<map<int, string>::iterator, bool> Insert_Pair;
	
    mapStudent.insert(pair<int, string>(1, "student_one"));
 
    mapStudent.insert(pair<int, string>(2, "student_two"));
 
    Insert_Pair=mapStudent.insert(pair<int, string>(3, "student_three"));
	cout << "ok:" << Insert_Pair.second << endl;
    map<int, string>::iterator iter;
 
    for(iter = mapStudent.begin(); iter != mapStudent.end(); iter++)
 
       cout<<iter->first<<' '<<iter->second<<endl;

}
int map2(){  
  
    map<int, string> mapStudent;  
  
    mapStudent.insert(map<int, string>::value_type (1, "student_one"));  
  
    mapStudent.insert(map<int, string>::value_type (2, "student_two"));  
  
    mapStudent.insert(map<int, string>::value_type (3, "student_three"));  
  
    map<int, string>::iterator iter;  
  
    for(iter = mapStudent.begin(); iter != mapStudent.end(); iter++)  
  
       cout<<iter->first<<' '<<iter->second<<endl;
  
}  
int map3() 
{  
    map<int, string> mapStudent;
  
    mapStudent[1] = "student_one";  
  
    mapStudent[2] = "student_two";  
  
    mapStudent[3] = "student_three"; 
	
	mapStudent[4] = "student_four"; 
  
	for(auto i:mapStudent){
		cout<<i.first<<' '<<i.second<<endl; 
	}
} 
int map3_1()  
  
{  
  
    map<string, string> mapStudent;
  
    mapStudent["one"] = "student_one";  
  
    mapStudent["two"] = "student_two";  
  
    mapStudent["three"] = "student_three"; 
	
	mapStudent["four"] = "student_four"; 
	
	for(auto i:mapStudent){
		cout<<i.first<<' '<<i.second<<endl; 
	}
} 
void future0(){
	std::future<int> future = std::async(std::launch::async, [](){ 
		std::this_thread::sleep_for(std::chrono::seconds(3));
		return 8;  
	});
	std::cout << "waiting...\n";
	std::future_status status;
	do {
		status = future.wait_for(std::chrono::seconds(1));
		if (status == std::future_status::deferred) {
			std::cout << "deferred\n";
		} else if (status == std::future_status::timeout) {
			std::cout << "timeout\n";
		} else if (status == std::future_status::ready) {
			std::cout << "ready!\n";
		}
	} while (status != std::future_status::ready); 
	std::cout << "result is " << future.get() << '\n';
}
#include <functional>

int hash0 ()
{
  char nts1[] = "Test11111111111";
  char nts2[] = "Test";
  std::string str1 (nts1);
  std::string str2 (nts2);

  std::hash<char*> ptr_hash;
  std::hash<std::string> str_hash;
	cout << "nts1 hash val:" << str_hash(str1) << "\n";
	cout << "nts2 hash val:" << str_hash(str2) << "\n";
  std::cout << "same hashes:\n" << std::boolalpha;
  std::cout << "nts1 and nts2: " << (ptr_hash(nts1)==ptr_hash(nts2)) << '\n';
  std::cout << "str1 and str2: " << (str_hash(str1)==str_hash(str2)) << '\n';

  return 0;
}
#define MacMakeNode(n,x,l,r) ({\
	auto n = makeNode(x,l,r);\
	if(!n){\
		s_err("makeNode "#n" failed!!");\
		return nullptr;\
	}\
	n;\
})
struct TreeNode{
	int val;
	TreeNode* left;
	TreeNode* right;
	TreeNode(int x):val(x),left(nullptr),right(nullptr){}
};


class Solution{
	void getSubPathSum(TreeNode* root,int sum,vector<int> path,vector<vector<int>>& pathVct){
		size_t subNum = 0;
		path.push_back(root->val);
		if(root->left){
			subNum ++;
			getSubPathSum(root->left,sum,path,pathVct);	
		}
		if(root->right){
			subNum ++;
			if(subNum > 1){
				vector<int> nPath(path);
				getSubPathSum(root->right,sum,nPath,pathVct);
			}else{
				getSubPathSum(root->right,sum,path,pathVct);
			}					
		}
		if(subNum < 1){
			int pathSum = 0;
			printf("[ ");
			for(auto i:path){
				pathSum += i;
				printf("%02d ",i);
			}
			printf("]\n");
			if(pathSum == sum){
				pathVct.push_back(path);
			}
			path.clear();
		}		
	}
	TreeNode* makeNode(int val,TreeNode* left = nullptr,TreeNode* right = nullptr){
		auto node = new TreeNode(val);
		if(!node){
			s_err("oom");
			return nullptr;
		}
		if(left){
			node->left = left;
		}
		if(right){
			node->right = right;
		}
		return node;
	}
	TreeNode* createTree(){
		TreeNode* n4_1 = MacMakeNode(n4_1,7,nullptr,nullptr);
		TreeNode* n4_2 = MacMakeNode(n4_2,2,nullptr,nullptr);
		TreeNode* n4_3 = MacMakeNode(n4_3,5,nullptr,nullptr);
		TreeNode* n4_4 = MacMakeNode(n4_4,1,nullptr,nullptr);
		TreeNode* n3_1 = MacMakeNode(n3_1,11,n4_1,n4_2);
		TreeNode* n3_2 = MacMakeNode(n3_2,13,nullptr,nullptr);
		TreeNode* n3_3 = MacMakeNode(n3_3,4,n4_3,n4_4);
		TreeNode* n2_1 = MacMakeNode(n2_1,4,n3_1,nullptr);
		TreeNode* n2_2 = MacMakeNode(n2_2,8,n3_2,n3_3);
		TreeNode* n1_1 = MacMakeNode(n1_1,5,n2_1,n2_2);
		return n1_1;
	}	
	void showTree(TreeNode* root){
		if(!root){
			return;
		}
		printf("%02d ",root->val);
		if(root->left){
			showTree(root->left);
		}
		if(root->right){
			showTree(root->right);
		}
	}
	vector<vector<int>> pathDance(TreeNode* root,int sum){
		vector<vector<int>> pathVct;
		vector<int> path;
		getSubPathSum(root,sum,path,pathVct);
		s_war("");
		return pathVct;
	}
public:
	Solution(){
		vector<vector<int>> pathVct;
		TreeNode* root = createTree();
		if(!root){
			s_err("createTree failed!");
			return ;;
		}
		printf("raw:[ ");		
		showTree(root);
		printf("]\n");
		pathVct = pathDance(root,22);
		for(auto i:pathVct){
			printf("[ ");
			for(auto j:i){
				printf("%d ",j);
			}
			printf("]\n");
		}
	}
};
int main(int argc, char *argv[]) {
	std::string FuncStr;
	int opt = 0;
	while ((opt = getopt_long_only(argc, argv, "f:l:ph", NULL, NULL)) != -1) {
		switch (opt) {
		case 'l':
			lzUtils_logInit(optarg, NULL);
			break;
		case 'p':
			lzUtils_logInit(NULL, optarg);
			break;
		case 'f':
			FuncStr = optarg;
			break;
		default: /* '?' */
			s_err("no para!");
		}
	}
	if(0){		
	}else if(FuncStr == "map1"){
		map1();
	}else if(FuncStr == "map2"){
		map2();
	}else if(FuncStr == "map3"){
		map3();
	}else if(FuncStr == "map3_1"){
		map3_1();
	}else if(FuncStr == "future0"){
		future0();
	}else if(FuncStr == "hash0"){
		hash0();
	}else if(FuncStr == "solution"){
		Solution solution;
	}
	return 0;
}