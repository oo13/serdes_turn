# Build settings for Unit Test of prec_ctrl.
# Copyright © 2023 OOTA, Masato
# This is published under CC0 1.0.
# For more information, see CC0 1.0 Universal (CC0 1.0) at <https://creativecommons.org/publicdomain/zero/1.0/legalcode>.

project('serdes_turn', 'c',
    default_options: [
        'c_std=c89',
        'buildtype=release',
        'warning_level=3',
        'strip=true',
    ],
    license: 'GPL-3.0-or-later',
    # license_files: ['../LICENSE'],
)

compiler = meson.get_compiler('c')
m_dep = compiler.find_library('m', required: false)

srcs = [
    'source/serdes_turn_deg.c',
]
incdirs = ['source']

lib = library(
    'serdes_turn',
    srcs,
    include_directories: incdirs,
    dependencies: m_dep,
    install: true,
)

# install header
install_headers('source/serdes_turn_deg.h')

# install man
python3 = find_program('python3', required: false)
doxygen = find_program('doxygen', required: false)
if python3.found() and doxygen.found()
    man_func = [
        'serialize_turn_to_deg.3',
        'serialize_turn_to_deg_p.3',
        'serialize_turn_to_deg_ps.3',
        'deserialize_turn_from_deg.3',
        'conv_deg_to_turn.3',
    ]
    utils_dir = 'utils'

    generate_man_config_target = custom_target(
        'generate-doxygen-config-for-man',
        input: ['serdes_turn.doxygen'],
        output: ['serdes_turn_man.doxygen'],
        command: [
            python3,
            '@SOURCE_ROOT@' / utils_dir / 'conv_doxygen_config_for_man',
            '-s', '@SOURCE_ROOT@' / 'source',
            '-i', '@INPUT@',
            '-o', '@OUTPUT@',
        ],
    )
    generate_man_target = custom_target(
        'generate-man',
        depends: [generate_man_config_target],
        command: [
            python3,
            '@SOURCE_ROOT@' / utils_dir / 'generate_man',
            '-e', doxygen.full_path(),
            '-c', 'serdes_turn_man.doxygen',
            '@OUTPUT@',
        ],
        output: man_func,
        install: true,
        install_dir: 'man' / 'man3',
        install_tag: 'man',
    )
endif


# For Test
test_srcs = [
    'test/test_serdes_turn_deg.c',
]

test_opts = []
if get_option('test-analyzer')
   test_opts += ['-fanalyzer']
endif
test_opt_sanitize = []
if get_option('test-sanitize-address')
   test_opts += ['-fsanitize=address']
endif
if get_option('test-sanitize-undefined')
   test_opts += ['-fsanitize=undefined']
endif

test_opts = compiler.get_supported_arguments(test_opts)

# C89
test_lib = static_library(
    'test_serdes_turn_deg_c89',
    srcs,
    include_directories: incdirs,
    c_args: test_opts,
    override_options: [
        'buildtype=debugoptimized'
    ],
)
# C99
test_exe = executable(
    'test_serdes_turn_deg',
    test_srcs,
    include_directories: incdirs,
    link_with: test_lib,
    dependencies: m_dep,
    c_args: test_opts,
    link_args: test_opts,
    override_options: [
        'c_std=c99',
        'buildtype=debugoptimized'
    ],
)
test('Unit Test', test_exe)
