
{
    "targets": [
        {
            "target_name": "plugin",
            "include_dirs": [
                "include"
            ],
            "libraries": ["-lopencv_core", "-lopencv_imgproc", "-lopencv_objdetect", "-lopencv_video", "-lopencv_dnn"],
            # "link_settings": {"libraries": ["-L<(oci_lib_dir)/lib"]},
            "sources": ["../core_bindings.cpp",
                        "binding.cc"],
            "conditions": [
                ["OS==\"linux\"", {"cflags": ["-frtti", "-w"],
                                   "cflags_cc": ["-frtti", "-w"]
                                   }]
            ]
        }
    ]
}
