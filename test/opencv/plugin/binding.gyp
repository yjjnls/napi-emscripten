
{
    "targets": [
        {
            "target_name": "plugin",
            "include_dirs": [
                "./../../../include"
            ],
            "libraries": ["-lopencv_core", "-lopencv_imgproc", "-lopencv_objdetect", "-lopencv_video", "-lopencv_dnn"],
            # "link_settings": {"libraries": ["-L<(oci_lib_dir)/lib"]},
            "sources": ["binding.cc"],
            "conditions": [
                ["OS==\"linux\"", {"cflags": ["-O2", "-std=c++11", "-w","-Dopencv_String=1"],
                                   "cflags_cc": ["-O2", "-std=c++11", "-w","-Dopencv_String=1"]
                                   }]
            ]
        }
    ]
}
