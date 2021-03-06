/*
 * Distributed under the OSI-approved Apache License, Version 2.0.  See
 * accompanying file Copyright.txt for details.
 *
 * CompressLibPressio.cpp
 *
 *  Created on: Tue Apr 13, 2021
 *      Author: Robert Underwood robertu@g.clemson.edu
 */

#include "CompressLibPressio.h"

#include <cmath>     //std::ceil
#include <ios>       //std::ios_base::failure
#include <stdexcept> //std::invalid_argument

extern "C" {
#include "libpressio.h"
}

#include "adios2/helper/adiosFunctions.h"

namespace adios2
{
namespace core
{
namespace compress
{

static pressio_dtype adios_to_libpressio_dtype(DataType var_type)
{
    if (var_type == helper::GetDataType<float>())
    {
        return pressio_float_dtype;
    }
    if (var_type == helper::GetDataType<double>())
    {
        return pressio_double_dtype;
    }
    if (var_type == helper::GetDataType<int8_t>())
    {
        return pressio_int8_dtype;
    }
    if (var_type == helper::GetDataType<int16_t>())
    {
        return pressio_int16_dtype;
    }
    if (var_type == helper::GetDataType<int32_t>())
    {
        return pressio_int32_dtype;
    }
    if (var_type == helper::GetDataType<int64_t>())
    {
        return pressio_int64_dtype;
    }
    if (var_type == helper::GetDataType<uint8_t>())
    {
        return pressio_uint8_dtype;
    }
    if (var_type == helper::GetDataType<uint16_t>())
    {
        return pressio_uint16_dtype;
    }
    if (var_type == helper::GetDataType<uint32_t>())
    {
        return pressio_uint32_dtype;
    }
    if (var_type == helper::GetDataType<uint64_t>())
    {
        return pressio_uint64_dtype;
    }
    throw std::runtime_error("libpressio: unexpected datatype");
}

static std::vector<size_t> adios_to_libpressio_dims(Dims const &dims)
{
    std::vector<size_t> lp_dims(std::rbegin(dims), std::rend(dims));
    return lp_dims;
}

struct pressio_param
{
    enum class type
    {
        early,
        late,
        unset,
        malformed
    } type = type::unset;
    bool has_index = false;
    size_t index = 0;
    std::string name;
};

pressio_param parse_adios_config_entry(std::string const &key)
{
    pressio_param p;
    static const std::string early_config_prefix = "early:";
    static const std::string compressor_config_prefix = "config:";
    size_t current = 0;

    try
    {
        if (key.find(early_config_prefix) == 0)
        {
            p.type = pressio_param::type::early;
            current += early_config_prefix.size();
        }
        else if (key.find(compressor_config_prefix) == 0)
        {
            p.type = pressio_param::type::late;
            current += compressor_config_prefix.size();
        }
        else
        {
            p.type = pressio_param::type::unset;
        }
        if (current != 0)
        {
            if (key.at(current) == '[')
            {
                // we have an array entry
                p.has_index = true;
                auto digit_len = key.find_first_of(']', current);
                if (digit_len == std::string::npos)
                {
                    throw std::invalid_argument("invalid substr");
                }
                p.index =
                    stoll(key.substr(current + 1, digit_len - (current + 1)));
                current = digit_len + 2;
                if (key.at(digit_len + 1) != ':')
                {
                    throw std::invalid_argument("missing expected :");
                }
            }
            else
            {
                // we have a scalar entry
                p.has_index = false;
            }

            p.name = key.substr(current);
        }
    }
    catch (std::invalid_argument &)
    {
        p.type = pressio_param::type::malformed;
    }
    catch (std::out_of_range &)
    {
        p.type = pressio_param::type::malformed;
    }
    return p;
}

static pressio_compressor *adios_to_libpressio_compressor(Params const &params)
{
    pressio *instance = pressio_instance();
    auto compressor_it = params.find("compressor_id");
    if (compressor_it != params.end())
    {
        pressio_compressor *compressor =
            pressio_get_compressor(instance, compressor_it->second.c_str());
        pressio_release(instance);
        if (compressor == nullptr)
        {
            throw std::runtime_error("compressor unavailable: " +
                                     compressor_it->second);
        }

        // adios parameters have unique names and must have string type
        // use the syntax early:foobar to set parameter foobar "early"
        // use the syntax config:foobar to set parameter foobar "late"
        // we need unique names for array entries use
        // early_config:[(\d+)]:foobar to create an array
        std::map<std::string, std::map<size_t, std::string>> early, late;
        for (auto const &param : params)
        {
            auto parsed = parse_adios_config_entry(param.first);
            std::map<std::string, std::map<size_t, std::string>> *config =
                nullptr;
            switch (parsed.type)
            {
            case pressio_param::type::early:
                config = &early;
                break;
            case pressio_param::type::late:
                config = &late;
                break;
            case pressio_param::type::unset:
                continue;
            case pressio_param::type::malformed:
                pressio_compressor_release(compressor);
                throw std::runtime_error("malformed parameter name " +
                                         param.first);
            }

            if (parsed.has_index)
            {
                (*config)[parsed.name][parsed.index] = param.second;
            }
            else
            {
                (*config)[parsed.name][static_cast<size_t>(0)] = param.second;
            }
        }

        // convert early
        pressio_options *early_opts = pressio_options_new();
        for (auto const &entry : early)
        {
            if (entry.second.size() == 1)
            {
                pressio_options_set_string(
                    early_opts, entry.first.c_str(),
                    entry.second.find(static_cast<size_t>(0))->second.c_str());
            }
            else
            {
                std::vector<const char *> entries;
                std::transform(entry.second.cbegin(), entry.second.cend(),
                               std::back_inserter(entries),
                               [](decltype(*entry.second.begin()) s) {
                                   return s.second.c_str();
                               });
                pressio_options_set_strings(early_opts, entry.first.c_str(),
                                            entries.size(), entries.data());
            }
        }

        pressio_compressor_set_options(compressor, early_opts);
        pressio_options_free(early_opts);

        pressio_options *compressor_options =
            pressio_compressor_get_options(compressor);

        for (auto const &entry : late)
        {
            pressio_option *option = nullptr;
            if (entry.second.size() == 1)
            {
                option = pressio_option_new_string(
                    entry.second.find(static_cast<size_t>(0))->second.c_str());
            }
            else
            {
                std::vector<const char *> entries;
                std::transform(entry.second.begin(), entry.second.end(),
                               std::back_inserter(entries),
                               [](decltype(*entry.second.begin()) s) {
                                   return s.second.c_str();
                               });
                option =
                    pressio_option_new_strings(entries.data(), entries.size());
            }

            switch (pressio_options_cast_set(compressor_options,
                                             entry.first.c_str(), option,
                                             pressio_conversion_special))
            {
            case pressio_options_key_set:
                break;
            case pressio_options_key_exists:
                pressio_options_free(compressor_options);
                pressio_compressor_release(compressor);
                throw std::runtime_error("enable to convert " + entry.first);
            case pressio_options_key_does_not_exist:
                pressio_options_free(compressor_options);
                pressio_compressor_release(compressor);
                throw std::runtime_error("unexpected option " + entry.first);
            }
            pressio_option_free(option);
        }
        pressio_compressor_set_options(compressor, compressor_options);
        pressio_options_free(compressor_options);

        return compressor;
    }
    throw std::runtime_error("missing required \"compressor_id\" setting");
}

CompressLibPressio::CompressLibPressio(const Params &parameters)
: Operator("libpressio", parameters)
{
}

size_t CompressLibPressio::BufferMaxSize(const size_t sizeIn) const
{
    return static_cast<size_t>(std::ceil(1.1 * sizeIn) + 600);
}

size_t CompressLibPressio::Compress(const void *dataIn, const Dims &dimensions,
                                    const size_t /*elementSize*/,
                                    DataType varType, void *bufferOut,
                                    const Params &parameters,
                                    Params &info) const
{
    auto inputs_dims = adios_to_libpressio_dims(dimensions);
    pressio_data *input_buf = pressio_data_new_nonowning(
        adios_to_libpressio_dtype(varType), const_cast<void *>(dataIn),
        inputs_dims.size(), inputs_dims.data());
    pressio_data *output_buf =
        pressio_data_new_empty(pressio_byte_dtype, 0, nullptr);
    pressio_compressor *compressor = nullptr;
    try
    {
        compressor = adios_to_libpressio_compressor(parameters);
    }
    catch (std::exception &e)
    {
        pressio_data_free(input_buf);
        pressio_data_free(output_buf);
        throw;
    }

    if (pressio_compressor_compress(compressor, input_buf, output_buf) != 0)
    {
        pressio_data_free(input_buf);
        pressio_data_free(output_buf);
        throw std::runtime_error(std::string("pressio_compressor_compress: ") +
                                 pressio_compressor_error_msg(compressor));
    }

    size_t size_in_bytes = 0;
    void *bytes = pressio_data_ptr(output_buf, &size_in_bytes);
    memcpy(bufferOut, bytes, size_in_bytes);

    pressio_data_free(input_buf);
    pressio_data_free(output_buf);

    return static_cast<size_t>(size_in_bytes);
}

size_t CompressLibPressio::Decompress(const void *bufferIn, const size_t sizeIn,
                                      void *dataOut, const Dims &dimensions,
                                      DataType varType,
                                      const Params &params) const
{
    std::vector<size_t> dims = adios_to_libpressio_dims(dimensions);
    pressio_data *output_buf = pressio_data_new_owning(
        adios_to_libpressio_dtype(varType), dims.size(), dims.data());

    pressio_data *input_buf = pressio_data_new_nonowning(
        pressio_byte_dtype, const_cast<void *>(bufferIn), 1, &sizeIn);

    pressio_compressor *compressor = nullptr;
    try
    {
        compressor = adios_to_libpressio_compressor(params);
    }
    catch (std::exception &)
    {
        pressio_data_free(input_buf);
        pressio_data_free(output_buf);
        throw;
    }

    if (pressio_compressor_decompress(compressor, input_buf, output_buf) != 0)
    {
        pressio_data_free(input_buf);
        pressio_data_free(output_buf);
        throw std::runtime_error(
            std::string("pressio_compressor_decompress: ") +
            pressio_compressor_error_msg(compressor));
    }

    size_t size_in_bytes = 0;
    void *output = pressio_data_ptr(output_buf, &size_in_bytes);
    std::memcpy(dataOut, output, size_in_bytes);

    pressio_data_free(input_buf);
    pressio_data_free(output_buf);
    return size_in_bytes;
}

bool CompressLibPressio::IsDataTypeValid(const DataType type) const
{
#define declare_type(T)                                                        \
    if (helper::GetDataType<T>() == type)                                      \
    {                                                                          \
        return true;                                                           \
    }
    ADIOS2_FOREACH_LIBPRESSIO_TYPE_1ARG(declare_type)
#undef declare_type
    return false;
}

} // end namespace compress
} // end namespace core
} // end namespace adios2
