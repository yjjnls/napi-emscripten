#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>
#include <list>
#include <emscripten/bind.h>
#include <errno.h>
#ifdef USE_NODEFS
#include <emscripten/emscripten.h>
#endif

using namespace emscripten;
using namespace tesseract;

namespace binding_utils {
int Init(tesseract::TessBaseAPI &obj, const std::string &path, const std::string &lang)
{
    return obj.Init(path.c_str(), lang.c_str());
}
bool SetVariable(tesseract::TessBaseAPI &obj, const std::string &key, const std::string &value)
{
    return obj.SetVariable(key.c_str(), value.c_str());
}
void SetImage(tesseract::TessBaseAPI &obj, intptr_t pix)
{
    obj.SetImage(reinterpret_cast<Pix *>(pix));
}
std::string GetUTF8Text(tesseract::TessBaseAPI &obj)
{
    char *outText = obj.GetUTF8Text();
    std::string res(strlen(outText) + 1, 0);
    strcpy((char *)res.c_str(), outText);
    delete[] outText;
    return res;
}
intptr_t wrapper_pixRead(const std::string &filename)
{
    Pix *image = pixRead(filename.c_str());
    return reinterpret_cast<intptr_t>(image);
}
intptr_t wrapper_pixRead2(intptr_t pointer, size_t size)
{
    unsigned char *buffer = reinterpret_cast<unsigned char *>(pointer);
    Pix *image = pixReadMemBmp(buffer, size);
    return reinterpret_cast<intptr_t>(image);
}
void wrapper_pixDestroy(intptr_t image)
{
    pixDestroy(reinterpret_cast<Pix **>(&image));
}
std::string version()
{
    return tesseract::TessBaseAPI::Version();
}
void onload()
{
#ifdef USE_NODEFS
    EM_ASM({
        FS.mkdir("/fs");
        FS.mount(NODEFS, {root : '/'}, "/fs");
    });
#endif
}
void test_file(const std::string &filename)
{
    errno = 0;
    FILE *fp = NULL;
    fp = fopen(filename.c_str(), "rb");
    if (fp) {
        printf("Open file OK!\n");
    } else {
        printf("Error %d \n", errno);
        printf("The function fopen failed due to : [%s]\n", (char *)strerror(errno));
    }
}

}  // namespace binding_utils

EMSCRIPTEN_BINDINGS(binding_utils)
{
    emscripten::class_<tesseract::TessBaseAPI>("TessBaseAPI")
        .constructor<>()
        .function("Init", select_overload<int(tesseract::TessBaseAPI &, const std::string &, const std::string &)>(&binding_utils::Init))
        .function("SetVariable", select_overload<bool(tesseract::TessBaseAPI &, const std::string &, const std::string &)>(&binding_utils::SetVariable))
        .function("SetImage", select_overload<void(tesseract::TessBaseAPI &, intptr_t)>(&binding_utils::SetImage))
        .function("GetUTF8Text", select_overload<std::string(tesseract::TessBaseAPI &)>(&binding_utils::GetUTF8Text))
        .function("End", select_overload<void()>(&::tesseract::TessBaseAPI::End));

    function("pixRead", select_overload<intptr_t(const std::string &)>(&binding_utils::wrapper_pixRead));
    function("pixReadMemBmp", select_overload<intptr_t(intptr_t, size_t)>(&binding_utils::wrapper_pixRead2));
    function("pixDestroy", select_overload<void(intptr_t)>(&binding_utils::wrapper_pixDestroy));
    function("version", select_overload<std::string()>(&binding_utils::version));
    function("onload", select_overload<void()>(&binding_utils::onload));
    function("test_file", select_overload<void(const std::string &)>(&binding_utils::test_file));

    constant("PSM_OSD_ONLY", PageSegMode::PSM_OSD_ONLY);
}
