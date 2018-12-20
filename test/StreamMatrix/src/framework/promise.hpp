#ifndef STREAM_MATRIX_FRAMEWORK_PROMISE_HPP
#define STREAM_MATRIX_FRAMEWORK_PROMISE_HPP


#include <framework/common.hpp>

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
        , app_(nullptr)
    {
    }
    StreamMatrix *StreamMatrix()
    {
        return owner_;
    }

    void resolve()
    {
        if (!responsed_) {
            callback_(0, "");
            responsed_ = true;
        }
    }
    void resolve(const nlohmann::json &param)
    {
        if (!responsed_) {
            callback_(0, param.dump());
            responsed_ = true;
        }
    }
    void reject(const nlohmann::json &param)
    {
        if (!responsed_) {
            callback_(1, param.dump());
            responsed_ = true;
        }
    }
    void reject(const std::string &message)
    {
        if (!responsed_) {
            callback_(1, message);
            responsed_ = true;
        }
    }
    const nlohmann::json &data() const
    {
        return jdata_;
    }

    const nlohmann::json &meta() const
    {
        return jmeta_;
    }
    void *user_data()
    {
        return user_data_;
    }

 private:
    nlohmann::json jdata_;
    nlohmann::json jmeta_;
    bool responsed_;
    IApp *app_;
    StreamMatrix *owner_;
    callback callback_;
    void *user_data_;
};

#endif