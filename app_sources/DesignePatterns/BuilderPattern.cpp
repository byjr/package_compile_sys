#include <iostream>
#include <string>

using namespace std;

#define DELETE(pointer) delete pointer; pointer=nullptr

class IProduct { //抽象产品类
public:
	virtual string &getName() = 0;
	virtual void show() = 0;
};

class Product : public IProduct { //具体产品类
public:
	string &getName() override {
		return name;
	}
	void show() override {
		cout << "name==" << name << endl;
	}

private:
	string name; //要构建的具体产品名字（分为多个步骤才能构建完毕）
};

class IBuilder {
public:
	IBuilder() {}
	virtual ~IBuilder() {}

	virtual void buildPart1() = 0;
	virtual void buildPart2() = 0;
	virtual IProduct *getProduuct() = 0;
};

class Builder : public IBuilder { //构建具体结构，细节的具体实现或者说表示层
public:
	Builder(IProduct *product) : product(product) {}
	~Builder() {}

	void buildPart1() override {
		product->getName().append("buildPart1+");
	}
	void buildPart2() override {
		product->getName().append("buildPart2");
	}
	IProduct *getProduuct() override {
		return product;
	}

private:
	IProduct *product;
};

class Director { //构建者，用表示层的细节实现接口函数按照一定逻辑组合成某一款产品
public:
	Director() {
		cout << __func__ << "\n";
	}
	~Director() {
		cout << __func__ << "\n";
	}

	IProduct *construct(IBuilder *builder) { //传入指针唯一目的是对该对象进行构建，构建前后指针地址一直不变
		builder->buildPart1();
		builder->buildPart2();
		return builder->getProduuct();
	}
};

void doBuilderPattern() {
	IProduct *product = new Product(); //创建一个初始化的产品
	IBuilder *builder = new Builder(product); //创建构造器，传入产品对象地址准备构造具体产品和功能
	Director().construct(builder); //构建具体产品和功能
	product->show();
	DELETE(builder);
	DELETE(product);
}