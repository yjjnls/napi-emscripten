
{
    "targets": [
        {
            "target_name": "plugin",
            "include_dirs": [
                "./../../../../include"
            ],
            "libraries": ["-llept", "-ltesseract"],
            # "link_settings": {"libraries": ["-L<(oci_lib_dir)/lib"]},
            "sources": ["binding.cc"],
            "conditions": [
                ["OS==\"linux\"", {"cflags": ["-O2", "-std=c++11", "-w"],
                                   "cflags_cc": ["-O2", "-std=c++11", "-w"]
                                   }]
            ]
        }
    ]
}
