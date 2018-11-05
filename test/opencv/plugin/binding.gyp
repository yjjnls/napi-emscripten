
{
    "targets": [
        {
            "target_name": "plugin",
            "include_dirs": [
                "./include"
            ],
            "libraries": ["-lopencv_core", "-lopencv_imgproc", "-lopencv_objdetect", "-lopencv_video", "-lopencv_dnn"],
            # "link_settings": {"libraries": ["-L<(oci_lib_dir)/lib"]},
            "sources": ["../core_bindings.cpp",
                        "binding.cc"],
            "conditions": [
                ["OS==\"linux\"", {"cflags": ["-O2", "-std=c++11", "-w"],
                                   "cflags_cc": ["-O2", "-std=c++11", "-w"]
                                   }]
            ]
        }
    ]
}
