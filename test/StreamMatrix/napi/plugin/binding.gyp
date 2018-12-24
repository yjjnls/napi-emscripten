
{
    "targets": [
        {
            "target_name": "plugin",
            "include_dirs": [
                "./../../../../include",
                "./../../src",
                "./../../src/utils",
                "/root/gstreamer/linux_x86_64/include/gstreamer-1.0",
                "/root/gstreamer/linux_x86_64/include/glib-2.0",
                "/root/gstreamer/linux_x86_64/lib/glib-2.0/include"
            ],
            "libraries": ["/root/Desktop/del/napi-emscripten/test/StreamMatrix/build/libstream_matrix.so", "-lgstreamer-1.0","-lgstbase-1.0","-lglib-2.0"],
            # "link_settings": {"libraries": ["-L../build"]},
            "sources": ["/binding.cc"],
            "conditions": [
                ["OS==\"linux\"", {"cflags": ["-O2", "-std=c++11", "-w"],
                                   "cflags_cc": ["-O2", "-std=c++11", "-w"]
                                   }]
            ]
        }
    ]
}
