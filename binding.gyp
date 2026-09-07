{
  "targets": [
    {
      "target_name": "node-sitef",
      "cflags!": [ "-fno-exceptions" ],
      "cflags_cc!": [ "-fno-exceptions" ],
      "msvs_settings": {
        "VCCLCompilerTool": { "ExceptionHandling": 1 },
      },
      "conditions": [
        [
          'OS=="win"',
          {
            "defines": [
              "_HAS_EXCEPTIONS=1"
              ]
          }
        ]
      ],
      # lib/promises/promises.hpp da um #include em cada promise .cpp, ou seja o
      # addon inteiro e uma unica unidade de traducao. Listar lib/promises/*
      # aqui compilaria cada arquivo outra vez e duplicaria todos os simbolos.
      "sources": [
        "lib/nodesitef.cpp"
      ],
      "include_dirs": [
        "<!@(node -p \"require('node-addon-api').include\")"
      ],
      'defines': [ 'NAPI_DISABLE_CPP_EXCEPTIONS' ],
    }
  ]
}
