#ifndef TRAPH_NN_MODULE_H_
#define TRAPH_NN_MODULE_H_

#include <memory>
#include <vector>

#include <traph/nn/variable.h>
#include <traph/nn/operation.h>

namespace traph
{
    class Module
    {
    private:
        std::vector<std::shared_ptr<VariableInterface>> parameters;
    public:
        virtual std::vector<TensorBasePtr<f32>> backward(TensorBasePtr<f32> output_grad)
        {
            
        }
    };
} // traph

#endif