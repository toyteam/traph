#include <traph/tensor/int_tensor.h>

namespace traph
{
    // definition
    // private
    void Tensor<i32>::auto_strides()
    {
        idx_type dim_num = _dimensions.size();
        _strides.resize(dim_num);
        idx_type stride = 1;
        if(_order == layout_type::column_major)
        {
            for (idx_type i = dim_num - 1; i >= 0; --i)
            {
                _strides[i] = stride;
                stride *= _dimensions[i];
            }
        }
        else
        {
            for (idx_type i = 0; i < dim_num; ++i)
            {
                _strides[i] = stride;
                stride *= _dimensions[i];
            }
        }
    }

    void Tensor<i32>::apply_impl(idx_type dim, idx_type idx, std::function<i32(i32)> f)
    {
        idx_type dim_size = _dimensions.size();

        idx_type step_len = _strides[dim];
        idx_type step_num = _dimensions[dim];
        
        for(idx_type i = 0; i < step_num; ++i)
        {
            if(dim == dim_size - 1)
                _rep->data[idx] = f(_rep->data[idx]);
            else
                apply_impl(dim + 1, idx, f);
            idx += step_len;
        }
    }

    void Tensor<i32>::reduce_impl(i32& result, idx_type dim, idx_type idx, std::function<i32(i32,i32)> f) const
    {
        idx_type dim_size = _dimensions.size();

        idx_type step_len = _strides[dim];
        idx_type step_num = _dimensions[dim];

        for(idx_type i = 0; i < step_num; ++i)
        {
            if(dim == dim_size - 1)
                result = f(result, _rep->data[idx]);
            else
                reduce_impl(result, dim + 1, idx, f);
            idx += step_len;
        }
    }

    i32 Tensor<i32>::reduce_dim_kernel(idx_type begin, idx_type step_len, idx_type step_num, std::function<i32(i32,i32)> f) const
    {
        i32 result{};
        for(idx_type i = 0; i < step_num; ++i)
        {
            result = f(result, _rep->data[begin]);
            begin += step_len;
        }
        return result;
    }

    void Tensor<i32>::reduce_dim_impl(Tensor<i32>& result, idx_type dim, idx_type reduce_dim,
        idx_type this_idx, idx_type result_idx,
        std::function<i32(i32,i32)> f) const
    {
        idx_type dim_size = _dimensions.size();

        if(dim == dim_size)
        {
            result._rep->data[result_idx] = 
                reduce_dim_kernel(this_idx, _strides[reduce_dim], _dimensions[reduce_dim], f);
            return;
        }

        if(dim == reduce_dim)
        {
            reduce_dim_impl(result, dim + 1, reduce_dim, this_idx,result_idx, f);
        }
        else
        {
            for(idx_type i = 0; i < _dimensions[dim]; ++i)
            {
                reduce_dim_impl(result, dim + 1, reduce_dim, this_idx,result_idx, f);
                    
                this_idx += _strides[dim];
                result_idx += result._strides[dim];
            }
        }
    }
    // public
    Tensor<i32>::Tensor()
        :_rep(new TensorStorage<i32>),
        _dimensions(), _offset(0), _strides(), _order(layout_type::column_major)
    {
    }

    Tensor<i32>::Tensor(const DimVector& dimensions)
        :_rep(new TensorStorage<i32>),
        _dimensions(dimensions), _offset(0), _strides(), _order(layout_type::column_major)
    {
        auto_strides();
        
        _rep->resize_(_dimensions.flat_size());
    }

    Tensor<i32>::Tensor(const DimVector& dimensions, layout_type order)
        :_rep(new TensorStorage<i32>),
        _dimensions(dimensions), _offset(0), _strides(), _order(order)
    {
        auto_strides();

        _rep->resize_(_dimensions.flat_size());
    }

    Tensor<i32>::Tensor(const DimVector& dimensions, const DimVector& strides)
        :_rep(new TensorStorage<i32>),
        _dimensions(dimensions), _offset(0), _strides(strides), _order(layout_type::column_major)
    {
        auto_strides();

        _rep->resize_(_dimensions.flat_size());
    }

    Tensor<i32>::Tensor(const DimVector& dimensions, const DimVector& strides, layout_type order)
        :_rep(new TensorStorage<i32>),
        _dimensions(dimensions), _offset(0), _strides(strides), _order(order)
    {
        auto_strides();

        _rep->resize_(_dimensions.flat_size());
    }

    Tensor<i32>::Tensor(const i32& t)
        :_rep(new TensorStorage<i32>),
        _dimensions(), _offset(0), _strides()
    {
        _dimensions.resize(1);
        auto_strides();
    }

    void Tensor<i32>::add_(TensorInterfacePtr other)
    {
		// check tensor other type

		// check broadcast.shape = this.shape

		// ok, get lhs, rhs
		Tensor<i32> * lhs = this;
		Tensor<i32> * rhs = dynamic_cast<Tensor<i32> *>(other.get());
		std::function<void(Tensor<i32> *, Tensor<i32> *, idx_type, idx_type,idx_type, idx_type)> add_impl =
			[&](Tensor<i32> * lhs, Tensor<i32> * rhs, idx_type lhs_dim, idx_type rhs_dim, idx_type lhs_idx, idx_type rhs_idx) {

			auto lhs_storage = std::dynamic_pointer_cast<TensorStorage<i32>>(lhs->storage())->data_ptr();
			auto rhs_storage = std::dynamic_pointer_cast<TensorStorage<i32>>(rhs->storage())->data_ptr();

			if (lhs_dim < -(lhs->size().size()) && rhs_dim < -(rhs->size().size()))
			{
				lhs_storage[lhs_idx] += rhs_storage[rhs_idx];
				return;
			}

			idx_type lsh_shape_size = lhs_dim >= -(lhs->size().size())? lhs->size(lhs_dim) : 1;
			idx_type rsh_shape_size = rhs_dim >= -(rhs->size().size()) ? rhs->size(rhs_dim) : 1;
			idx_type max_shape_size = std::max(lsh_shape_size, rsh_shape_size);

			for (idx_type i = 0; i < max_shape_size; ++i)
			{
				add_impl(lhs, rhs, lhs_dim - 1, rhs_dim - 1, lhs_idx, rhs_idx);

				if(lsh_shape_size > 1)
					lhs_idx += lhs->stride(lhs_dim);
				if (rsh_shape_size > 1)
					rhs_idx += rhs->stride(rhs_dim);
			}
		};

		add_impl(lhs, rhs, -1, -1, lhs->offset(), rhs->offset());
    }

    void Tensor<i32>::apply_(std::function<i32(i32)> f)
    {
        apply_impl(0, _offset, f);
    }

    TensorInterfacePtr Tensor<i32>::clone() const
    {
        std::shared_ptr<Tensor<i32>> cloned_tensor(new Tensor<i32>);
        cloned_tensor->_rep = std::dynamic_pointer_cast<TensorStorage<i32>>(_rep->clone());
        cloned_tensor->_dimensions = _dimensions;
        cloned_tensor->_offset = _offset;
        cloned_tensor->_strides = _strides;
        cloned_tensor->_order = _order;
        
        return cloned_tensor;
    }

    void Tensor<i32>::cos_()
    {
        apply_([](i32 a)->i32 {return std::cos(a); });
    }

    std::shared_ptr<TensorBase<f32>> Tensor<i32>::create_grad()
    {
        return std::shared_ptr<TensorBase<f32>>(new Tensor<f32>(_dimensions));
    }

	i32* Tensor<i32>::data_ptr()
    {
        return _rep->data_ptr();
    }

    const i32* Tensor<i32>::data_ptr() const
    {
        return _rep->data_ptr();
    }

    device_id Tensor<i32>::device() { return 0; }

	std::shared_ptr<TensorInterface> Tensor<i32>::inverse() const
	{
		throw std::runtime_error("No implement");
	}

    void Tensor<i32>::fill_(i32 value)
    {
        apply_([&value](i32 a)->i32 {return value; });
    }

	i32 Tensor<i32>::item() const
    {
        if(_dimensions.flat_size() == 1)
        {
            return _rep->data[_offset];
        }
        else
        {
            throw std::runtime_error("item: only one element tensors can be converted to scalars");
        }
    }

	std::shared_ptr<TensorInterface> Tensor<i32>::matmul(std::shared_ptr<TensorInterface> mat) const
	{
		auto right_matrix = std::dynamic_pointer_cast<Tensor<i32>>(mat);
		return matmul_impl(*this, *right_matrix);
	}

    idx_type Tensor<i32>::offset() const { return _offset; }

    layout_type Tensor<i32>::order() const { return _order; }

    platform_type Tensor<i32>::platform() { return platform_type::none; }

	i32 Tensor<i32>::reduce_(std::function<i32(i32, i32)> f) const
    {
		i32 result{};
        reduce_impl(result, 0, _offset, f);
        return result;
    }
    
    TensorInterfacePtr Tensor<i32>::reduce_dim(idx_type dim, std::function<i32(i32, i32)> f) const
    {
        DimVector reduced_dim = _dimensions;
        reduced_dim.erase(dim); // check dim?
        TensorBasePtr<i32> result(new Tensor<i32>(reduced_dim));
        TensorPtr<i32> raw_result = std::dynamic_pointer_cast<Tensor<i32>>(result);
        reduce_dim_impl(*(raw_result.get()), 0, dim, _offset, raw_result->_offset, f);
        return std::dynamic_pointer_cast<TensorInterface>(result);
    }
    
    void Tensor<i32>::reshape_(const DimVector& dims)
    {

    }
    
    void Tensor<i32>::resize_(const DimVector& dims)
    {
        _dimensions = dims;
        _rep->resize_(dims.flat_size());
        auto_strides();
    }

	std::shared_ptr<TensorInterface> Tensor<i32>::select(const SliceVector& slice) const
	{
		std::shared_ptr<Tensor<i32>> result(new Tensor<i32>);
		result->_rep = _rep;

		// dimension
		DimVector dim;
		std::fesetround(FE_TONEAREST);
		for (idx_type i = 0; i < slice.size(); ++i)
		{
			auto& each = slice[i];
			dim.push_back(
				std::lrint(std::ceil((each.end.value_or(_dimensions[i]) - each.start.value_or(0)) / (float)each.step.value_or(1)))
			);
		}
		result->_dimensions = dim;

		// offset
		idx_type new_offset = 1;
		for (idx_type i = 0; i < slice.size(); ++i)
		{
			new_offset *= _strides[i] * slice[i].start.value_or(0);
		}
		result->_offset = _offset + new_offset;

		// strides
		DimVector strides;
		for (idx_type i = 0; i < slice.size(); ++i)
		{
			strides.push_back(_strides[i] * slice[i].step.value_or(1));
		}
		result->_strides = strides;

		result->_order = _order;

		return std::dynamic_pointer_cast<TensorInterface>(result);
	}
    
    void Tensor<i32>::sin_()
    {
        apply_([](i32 a)->i32 {return std::sin(a); });
    }
    
    DimVector Tensor<i32>::size() const { return _dimensions;}
	
	idx_type Tensor<i32>::size(idx_type i) const
	{ 
		auto shape_size = _dimensions.size();
		if (i >= 0 && i < _dimensions.size())
			return _dimensions[i];
		else if (i <= -1 && i >= -_dimensions.size())
			return _dimensions[shape_size + i];
		else
			throw std::runtime_error("Dimension out of range");
	}
    
	std::shared_ptr<StorageBase<i32>>  Tensor<i32>::storage() const { return _rep; }
    
    DimVector Tensor<i32>::stride() const { return _strides; }
	
	idx_type Tensor<i32>::stride(idx_type i) const
	{
		auto stride_size = _strides.size();
		if (i >= 0 && i < _strides.size())
			return _strides[i];
		else if (i <= -1 && i >= -_strides.size())
			return _strides[stride_size + i];
		else
			throw std::runtime_error("Stride out of range");
	}
    
    TensorInterfacePtr Tensor<i32>::sum() const
    {
        DimVector d(1);
        d[0] = 1;

        TensorPtr<i32> result(new Tensor<i32>(d));
        result->_rep->data[0] = reduce_([](i32 a, i32 b)->i32 {return a + b; });
        return std::dynamic_pointer_cast<TensorInterface>(result);
    }
    
    std::string Tensor<i32>::to_string() const
    {
        std::function<std::string(const Tensor<i32>&, idx_type, idx_type)> to_string_impl =
			[&](const Tensor<i32>& t, idx_type dim, idx_type idx)->std::string {
            std::string result;
			if (dim == t.size().size())
            {
                result += std::to_string(t.data_ptr()[idx]);
				return result;
            }

			for (idx_type i = 0; i < t.size(dim); ++i)
			{
				if (dim != t.size().size() - 1 && i != 0) result += ",\n";
				if(dim != t.size().size() - 1)	result += "[";
				result += to_string_impl(t, dim + 1, idx);
				if (i != t.size(dim) - 1 && dim == t.size().size() - 1)
					result += ",";
				if (dim != t.size().size() - 1) result += "]";

				idx += t.stride(dim);
			}

			return result;
		};

		std::string result;
		result += "[" + to_string_impl(*this, 0, offset()) + "]";
		return result;
    }

    void Tensor<i32>::transpose_(idx_type dim0, idx_type dim1)
    {

    }

    std::shared_ptr<TensorInterface> Tensor<i32>::transpose(idx_type dim0, idx_type dim1)
    {

    }
}