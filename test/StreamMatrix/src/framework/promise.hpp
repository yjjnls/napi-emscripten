#ifndef STREAM_MATRIX_FRAMEWORK_PROMISE_HPP
#define STREAM_MATRIX_FRAMEWORK_PROMISE_HPP


#include "common.hpp"

class StreamMatrix;
class IApp;
class Promise
{
 public:
    Promise(StreamMatrix *matrix,
            callback cb,
            void *user_data,
            const nlohmann::json &jmeta = nlohmann::json(),
            const nlohmann::json &jdata = nlohmann::json())
        : owner_(matrix)
        , callback_(cb)
        , user_data_(user_data)
        , jdata_(jdata)
        , jmeta_(jmeta)
        , responsed_(false)
    {
    }
    StreamMatrix *StreamMatrix() { return owner_; }

 private:
    nlohmann::json jdata_;
    nlohmann::json jmeta_;
    bool responsed_;
    StreamMatrix *owner_;
    IApp *app_;
    callback callback_;
    void *user_data_;
};

#endif