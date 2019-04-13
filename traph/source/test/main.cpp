#include <algorithm>

#include <traph/core/tensor.h>
#include <traph/tensor/tensor.h>
#include <traph/nn/arithmetic.h>
#include <traph/core/variable.h>
#include <traph/nn/variable.h>

#include <iostream>

int main()
{
	/*
    traph::Tensor<float> a = traph::zeros<float>({4, 3});
    traph::Tensor<float> w = traph::ones<float>({3, 5});
    traph::Tensor<float> result = traph::matmul(a, w);
	*/
	// traph::Tensor<float> result2 = traph::add(a, 1.f);
	/*
	traph::Tensor<traph::f32> a = traph::zeros<traph::f32>({ 5000, 5000 });
	traph::Tensor<traph::f32> b = traph::zeros<traph::f32>({ 5000, 5000 });
	traph::Tensor<traph::f32> c = traph::matmul(a, b);
	*/

	/*
	auto a = traph::Variable<traph::f32>({2, 3});
	auto c = traph::mul(traph::mul(a, a), 3);
	auto out = traph::mean(c);
	out.backward();
	*/
	/*
	traph::Tensor<float> a = traph::ones<float>({ 10000, 10000 });

	auto b = a.sum();
	std::cout << b;
	*/
	// auto a = traph::Variable<traph::f32>({ 2, 3 });

	auto a = traph::ones<traph::f32>({ 2,3 });
	a->requires_grad_(true);
	auto b = traph::ones<traph::f32>({ 2,3 });
	auto c = traph::add<traph::f32>(a, b);
	auto d = traph::sum<traph::f32>(c);

	d->backward();

	std::cout << a->grad()->to_string();

    return 0;
}