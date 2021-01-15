#include <lzUtils/base.h>
#include <string>
#include <vector>
#include <getopt.h>
#include <unistd.h>
std::vector<int> g_arr;
#define swap(x,y) ({\
	int t=(x);\
	(x)=(y);\
	(y)=t;\
})
#define printf_arr(a,n) ({\
	s_war("Arr:");\
	for(int i=0;i<(n);i++){\
		printf("%02d,",a[i]);\
	}\
	printf("\n");\
})
void my_swap(int &x, int &y) {
	int t = x;
	x = y;
	y = t;
}
void BubSort(int *A, size_t n) {
	int i, j, m, t;
	for(i = n - 1; i > 0; i--) {
		for(j = 0; j < i; j++) {
			if(A[j] > A[j + 1]) {
				t = A[j];
				A[j] = A[j + 1];
				A[j + 1] = t;
			}
		}
	}
}
void InsterSort(int *a, size_t n) {
	int i, o, t;
	for(o = 1; o < n; o++) {
		t = a[o];
		i = o;
		while(i > 0 && a[i - 1] > t) {
			a[i] = a[i - 1];
			i--;
		}
		a[i] = t;
	}
}
// int A[]={89,3,4,6,7,9,2,1,4,3,66,44,33,11};
// {3,3,4,6,7,9,2,1,4,89,66,44,33,11};
// {3,3,4,6,7,9,2,1,4,89,66,44,33,11};

void QuickSort(int *s, int l, int r) {
	int i = l, j = r - 1, p = s[r], k;

	if(!(l < r))return;

	for(;;) {
		for(; s[j] > p;)j--;
		if(!(i < j))break;

		for(; s[i] < p;)i++;
		if(!(i < j))break;

		swap(s[i], s[j]);

		i++;
		j--;
		if(!(i < j))break;
	}

	k = s[i] < p ? (i + 1) : i;
	swap(s[k], s[r]);

	QuickSort(s, l, k - 1);
	QuickSort(s, k + 1, r);
}

void QuickSort1(int *a, int l, int r) {
	int i = l, j = r, p = a[i];
	if(!(l < r)) return ;

	do {
		for(; i < j && a[j] >= p; j--);
		if(!(i < j)) break ;
		a[i++] = a[j];

		for(; i < j && a[i] < p; i++);
		if(!(i < j)) break ;
		a[j--] = a[i];
	} while(i < j);

	a[i] = p;
	QuickSort1(a, l, i - 1);     	// 排序k左边
	QuickSort1(a, i + 1, r);    	// 排序k右边
}
void swap0() {
	int a[2] = {1, 2};
	printf_arr(a, 2);
	swap(a[0], a[1]);
	printf_arr(a, 2);
}
int get_rand() {
	return (rand() + 1) % 1000;
}
void update_arr(std::vector<int> &v) {
	v.clear();
	size_t size = get_rand();
	for(size_t i = 0; i < size; i++) {
		v.push_back(get_rand());
		// v[i] = (rand()+1)%100;
	}
}
bool check_arr(std::vector<int> &v) {
	for(size_t i = 1; i < v.size(); i++) {
		if(v[i] < v[i - 1]) {
			s_err("v[%d]=%d", i, v[i]);
			return false;
		}
	}
	return true;
}
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
	do {
		update_arr(g_arr);
		int *A = g_arr.data();
		size_t n = g_arr.size();
		s_raw("raw ");
		printf_arr(A, n);
		if(FuncStr == "BSort") {
			BubSort(A, n);
		} else if(FuncStr == "ISort") {
			InsterSort(A, n);
		} else if(FuncStr == "QSort") {
			QuickSort(A, 0, n - 1);
		} else if(FuncStr == "swap0") {
			swap0();
		} else if(FuncStr == "QSort1") {
			QuickSort1(A, 0, n - 1);
		}
		s_raw("new ");
		printf_arr(A, n);
		if(!check_arr(g_arr)) {
			return -1;
		}
	} while(1);

	return 0;
}