{
    "targets": [{
        "target_name": "warcraft-recorder-obs-engine",
        "cflags!": [ "-fno-exceptions" ],
        "cflags_cc!": [ "-fno-exceptions" ],
        "sources": [
            "src/main.cpp"
        ],
        'include_dirs': [
            "<!@(node -p \"require('node-addon-api').include\")",
            "include"
        ],
        'libraries': [
            "../bin/64bit/obs.lib"
        ],
        'dependencies': [
            "<!(node -p \"require('node-addon-api').gyp\")"
        ],
        'defines': [ 'NAPI_DISABLE_CPP_EXCEPTIONS' ],
        'conditions': [
            ['OS=="win"', {
                'copies': [{
                    'destination': '<(PRODUCT_DIR)',
                    'files': [
                        'bin/64bit/obs.dll'
                    ]
                }]
            }]
        ]
    }]
}