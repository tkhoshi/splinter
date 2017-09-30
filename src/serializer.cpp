/*
 * This file is part of the SPLINTER library.
 * Copyright (C) 2012 Bjarne Grimstad (bjarne.grimstad@gmail.com).
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "serializer.h"
#include <fstream>
#include "definitions.h"
#include "data_point.h"
#include <data_table.h>
#include <bspline.h>
#include <bspline_basis.h>
#include <bspline_basis_1d.h>
#include <knot_vector.h>

namespace SPLINTER
{

Serializer::Serializer()
{
    stream = StreamType(0);
}

Serializer::Serializer(const std::string &fileName)
{
    stream = StreamType(0);
    loadFromFile(fileName);
}

void Serializer::saveToFile(const std::string &fileName)
{
    std::fstream fs(fileName, std::fstream::out | std::fstream::binary);

    for (const auto& byte : stream)
        fs << byte;
}

void Serializer::loadFromFile(const std::string &fileName)
{
    // Open the file in binary mode at the end
    std::ifstream ifs(fileName, std::ios::binary | std::ios::ate);

    if (!ifs.is_open()) {
        std::string error_message("Serializer::loadFromFile: Unable to open file \"");
        error_message.append(fileName);
        error_message.append("\" for deserializing.");
        throw Exception(error_message);
    }

    // Because we opened the file at the end, tellg() will give us the size of the file
    std::ifstream::pos_type pos = ifs.tellg();

    std::vector<char> result(pos);

    ifs.seekg(0, std::ios::beg);

    // http://www.cplusplus.com/reference/vector/vector/data/
    // Elements of the vector are guaranteed to be stored in a contiguous array,
    // *result.data() can therefore be treated as an array of the same type as the vector
    ifs.read(result.data(), pos);
    //assert(ifs);

    stream.clear();
    // Convert from char to uint_8 vector
    for (const char& byte : result)
        stream.push_back((uint8_t)byte);

    read = stream.cbegin();
}

/*
 * get_size implementations
 */

size_t Serializer::get_size(const DataPoint &obj)
{
    return get_size(obj.x) + get_size(obj.y);
}

size_t Serializer::get_size(const DataTable &obj)
{
    return get_size(obj._allow_duplicates)
           + get_size(obj._allow_incomplete_grid)
           + get_size(obj._num_duplicates)
           + get_size(obj._dim_x)
           + get_size(obj._dim_y)
           + get_size(obj.samples)
           + get_size(obj.grid);
}

size_t Serializer::get_size(const BSpline &obj)
{
    return get_size(obj.basis)
           + get_size(obj.control_points)
           + get_size(obj.dim_x)
           + get_size(obj.dim_y);
}

size_t Serializer::get_size(const BSplineBasis &obj)
{
    return get_size(obj.bases)
           + get_size(obj.num_variables);
}

size_t Serializer::get_size(const BSplineBasis1D &obj)
{
    return get_size(obj.degree)
           + get_size(obj.knots)
           + get_size(obj.target_num_basis_functions);
}

size_t Serializer::get_size(const KnotVector &obj)
{
    return get_size(obj.knots);
}

size_t Serializer::get_size(const DenseMatrix &obj)
{
    size_t size = sizeof(obj.rows());
    size += sizeof(obj.cols());
    size_t numElements = obj.rows() * obj.cols();
    if (numElements > 0) {
        size += numElements * sizeof(obj(0,0));
    }
    return size;
}

size_t Serializer::get_size(const DenseVector &obj)
{
    size_t size = sizeof(obj.rows());
    size_t numElements = obj.rows();
    if (numElements > 0) {
        size += numElements * sizeof(obj(0));
    }
    return size;
}

size_t Serializer::get_size(const SparseMatrix &obj)
{
    DenseMatrix temp(obj);
    return get_size(temp);
}

size_t Serializer::get_size(const SparseVector &obj)
{
    DenseVector temp(obj);
    return get_size(temp);
}

/*
 * _serialize implementations
 */

void Serializer::_serialize(const DataPoint &obj)
{
    _serialize(obj.x);
    _serialize(obj.y);
}

void Serializer::_serialize(const DataTable &obj)
{
    _serialize(obj._allow_duplicates);
    _serialize(obj._allow_incomplete_grid);
    _serialize(obj._num_duplicates);
    _serialize(obj._dim_x);
    _serialize(obj._dim_y);
    _serialize(obj.samples);
    _serialize(obj.grid);
}

void Serializer::_serialize(const BSpline &obj)
{
    _serialize(obj.basis);
    _serialize(obj.control_points);
    _serialize(obj.dim_x);
    _serialize(obj.dim_y);
}

void Serializer::_serialize(const BSplineBasis &obj)
{
    _serialize(obj.bases);
    _serialize(obj.num_variables);
}

void Serializer::_serialize(const BSplineBasis1D &obj)
{
    _serialize(obj.degree);
    _serialize(obj.knots);
    _serialize(obj.target_num_basis_functions);
}

void Serializer::_serialize(const KnotVector &obj)
{
    _serialize(obj.knots);
}

void Serializer::_serialize(const DenseMatrix &obj)
{
    // Store the number of matrix rows and columns first
    _serialize(obj.rows());
    _serialize(obj.cols());
    // Store the matrix elements
    for (size_t i = 0; i < (size_t) obj.rows(); ++i) {
        for (size_t j = 0; j < (size_t) obj.cols(); ++j) {
            _serialize(obj(i,j));
        }
    }
}

void Serializer::_serialize(const DenseVector &obj)
{
    // Store the number of vector rows
    _serialize(obj.rows());
    // Store the vector elements
    for (size_t i = 0; i < (size_t) obj.rows(); ++i) {
        _serialize(obj(i));
    }
}

void Serializer::_serialize(const SparseMatrix &obj)
{
    DenseMatrix temp(obj);
    _serialize(temp);
}

void Serializer::_serialize(const SparseVector &obj)
{
    DenseVector temp(obj);
    _serialize(temp);
}

/*
 * deserialize implementations
 */

void Serializer::deserialize(DataPoint &obj)
{
    deserialize(obj.x);
    deserialize(obj.y);
}

void Serializer::deserialize(DataTable &obj)
{
    deserialize(obj._allow_duplicates);
    deserialize(obj._allow_incomplete_grid);
    deserialize(obj._num_duplicates);
    deserialize(obj._dim_x);
    deserialize(obj._dim_y);
    deserialize(obj.samples);
    deserialize(obj.grid);
}

void Serializer::deserialize(BSpline &obj)
{
    deserialize(obj.basis);
    deserialize(obj.control_points);
    deserialize(obj.dim_x);
    deserialize(obj.dim_y);
}

void Serializer::deserialize(BSplineBasis &obj)
{
    deserialize(obj.bases);
    deserialize(obj.num_variables);
}

void Serializer::deserialize(BSplineBasis1D &obj)
{
    deserialize(obj.degree);
    deserialize(obj.knots);
    deserialize(obj.target_num_basis_functions);
}

void Serializer::deserialize(KnotVector &obj)
{
    deserialize(obj.knots);
}

void Serializer::deserialize(DenseMatrix &obj)
{
    // Retrieve the number of rows
    size_t rows; deserialize(rows);
    size_t cols; deserialize(cols);

    obj.resize(rows, cols);

    for (size_t i = 0; i < rows; ++i)
    {
        for (size_t j = 0; j < cols; ++j)
        {
            deserialize(obj(i, j));
        }
    }
}

void Serializer::deserialize(DenseVector &obj)
{
    // Retrieve the number of rows
    size_t rows; deserialize(rows);

    obj.resize(rows);

    for (size_t i = 0; i < rows; ++i)
    {
        deserialize(obj(i));
    }
}

void Serializer::deserialize(SparseMatrix &obj)
{
    DenseMatrix temp(obj);
    deserialize(temp);
    obj = temp.sparseView();
}

void Serializer::deserialize(SparseVector &obj)
{
    DenseVector temp(obj);
    deserialize(temp);
    obj = temp.sparseView();
}

} // namespace SPLINTER
