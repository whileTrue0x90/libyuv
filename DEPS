vars = {
  'chromium_git': 'https://chromium.googlesource.com',
  'chromium_revision': '7b15adda2c1732ea49504046783a4c3709a04cad',
  'swarming_revision': '4bd9152f8a975d57c972c071dfb4ddf668e02200',
  # Three lines of non-changing comments so that
  # the commit queue can handle CLs rolling lss
  # and whatever else without interference from each other.
  'lss_revision': 'e6527b0cd469e3ff5764785dadcb39bf7d787154',
  # Three lines of non-changing comments so that
  # the commit queue can handle CLs rolling catapult
  # and whatever else without interference from each other.
  'catapult_revision': '2de84c0d21c28eeb042dc4d6a1a88553f6d286da',
}

deps = {
  'src/build':
    Var('chromium_git') + '/chromium/src/build' + '@' + 'dab57cd8f2aaeeed7ef8849c8601f85d777ae074',
  'src/buildtools':
    Var('chromium_git') + '/chromium/buildtools.git' + '@' + '1be57dc4c2c68fa564ee258a3af0cf8003007edf',
  'src/testing':
    Var('chromium_git') + '/chromium/src/testing' + '@' + '95329923c25592911d25c88ae19ac6b15262b966',
  'src/third_party':
    Var('chromium_git') + '/chromium/src/third_party' + '@' + 'ab59a032a035a42a7138ee329466e550304c651c',
  'src/third_party/catapult':
    Var('chromium_git') + '/catapult.git' + '@' + Var('catapult_revision'),
  'src/third_party/colorama/src':
    Var('chromium_git') + '/external/colorama.git' + '@' + '799604a1041e9b3bc5d2789ecbd7e8db2e18e6b8',
  'src/third_party/googletest/src':
    Var('chromium_git') + '/external/github.com/google/googletest.git' + '@' + '247a3d8e5e5d403f7fcacdb8ccc71e5059f15daa',
  'src/third_party/libjpeg_turbo':
    Var('chromium_git') + '/chromium/deps/libjpeg_turbo.git' + '@' + 'a1750dbc79a8792dde3d3f7d7d8ac28ba01ac9dd',
  'src/third_party/yasm/source/patched-yasm':
    Var('chromium_git') + '/chromium/deps/yasm/patched-yasm.git' + '@' + 'b98114e18d8b9b84586b10d24353ab8616d4c5fc',
  'src/tools':
    Var('chromium_git') + '/chromium/src/tools' + '@' + 'dbb32b161ba02f538d55e702d1811f81ea157bbd',
  'src/tools/gyp':
    Var('chromium_git') + '/external/gyp.git' + '@' + 'd61a9397e668fa9843c4aa7da9e79460fe590bfb',
   'src/tools/swarming_client':
     Var('chromium_git') + '/infra/luci/client-py.git' + '@' +  Var('swarming_revision'),

  # libyuv-only dependencies (not present in Chromium).
  'src/third_party/gflags':
    Var('chromium_git') + '/external/webrtc/deps/third_party/gflags' + '@' + '892576179b45861b53e04a112996a738309cf364',
  'src/third_party/gflags/src':
    Var('chromium_git') + '/external/github.com/gflags/gflags' + '@' + '03bebcb065c83beff83d50ae025a55a4bf94dfca',
  'src/third_party/gtest-parallel':
    Var('chromium_git') + '/external/webrtc/deps/third_party/gtest-parallel' + '@' + '1dad0e9f6d82ff994130b529d7d814b40eb32b0e',

  'src/third_party/lss': {
    'url': Var('chromium_git') + '/linux-syscall-support.git' + '@' + Var('lss_revision'),
    'condition': 'checkout_android or checkout_linux',
  },

  # Android deps:
  'src/base': {
    'url': Var('chromium_git') + '/chromium/src/base' + '@' + '6e1f9013c65118fcf71ec258bf796b445067a730',
    'condition': 'checkout_android',
  },
  'src/third_party/android_ndk': {
    'url': Var('chromium_git') + '/android_ndk.git' + '@' + 'e951c37287c7d8cd915bf8d4149fd4a06d808b55',
    'condition': 'checkout_android',
  },
  'src/third_party/android_tools': {
    'url': Var('chromium_git') + '/android_tools.git' + '@' + 'a2e9bc7c1b41d983577907df51d339fb1e0fd02f',
    'condition': 'checkout_android',
  },
  'src/third_party/ced/src': {
    'url': Var('chromium_git') + '/external/github.com/google/compact_enc_det.git' + '@' + '94c367a1fe3a13207f4b22604fcfd1d9f9ddf6d9',
    'condition': 'checkout_android',
  },
  'src/third_party/icu': {
    'url': Var('chromium_git') + '/chromium/deps/icu.git' + '@' + '94d819fa3e7e71f3dd8210d428c213ddb6e2b336',
    'condition': 'checkout_android',
  },
  'src/third_party/jsr-305/src': {
    'url': Var('chromium_git') + '/external/jsr-305.git' + '@' + '642c508235471f7220af6d5df2d3210e3bfc0919',
    'condition': 'checkout_android',
  },
  'src/third_party/junit/src': {
    'url': Var('chromium_git') + '/external/junit.git' + '@' + '64155f8a9babcfcf4263cf4d08253a1556e75481',
    'condition': 'checkout_android',
  },
  'src/third_party/mockito/src': {
    'url': Var('chromium_git') + '/external/mockito/mockito.git' + '@' + 'de83ad4598ad4cf5ea53c69a8a8053780b04b850',
    'condition': 'checkout_android',
  },
  'src/third_party/requests/src': {
    'url': Var('chromium_git') + '/external/github.com/kennethreitz/requests.git' + '@' + 'f172b30356d821d180fa4ecfa3e71c7274a32de4',
    'condition': 'checkout_android',
  },
  'src/third_party/robolectric/robolectric': {
    'url': Var('chromium_git') + '/external/robolectric.git' + '@' + '7e067f1112e1502caa742f7be72d37b5678d3403',
    'condition': 'checkout_android',
  },
  'src/third_party/ub-uiautomator/lib': {
    'url': Var('chromium_git') + '/chromium/third_party/ub-uiautomator.git' + '@' + '00270549ce3161ae72ceb24712618ea28b4f9434',
    'condition': 'checkout_android',
  },

  # iOS deps:
  'src/ios': {
    'url': Var('chromium_git') + '/chromium/src/ios' + '@' + 'a510a0c751beb7f29ed699c581d2321f67661246',
    'condition': 'checkout_ios'
  },

  # Win deps:
  # Dependencies used by libjpeg-turbo
  'src/third_party/yasm/binaries': {
    'url': Var('chromium_git') + '/chromium/deps/yasm/binaries.git' + '@' + '52f9b3f4b0aa06da24ef8b123058bb61ee468881',
    'condition': 'checkout_win',
  },
}

# Define rules for which include paths are allowed in our source.
include_rules = [ '+gflags' ]

pre_deps_hooks = [
  {
    # Remove any symlinks from before 177567c518b121731e507e9b9c4049c4dc96e4c8.
    # TODO(kjellander): Remove this in March 2017.
    'name': 'cleanup_links',
    'pattern': '.',
    'action': ['python', 'src/cleanup_links.py'],
  },
]

hooks = [
  {
    # This clobbers when necessary (based on get_landmines.py). It should be
    # an early hook but it will need to be run after syncing Chromium and
    # setting up the links, so the script actually exists.
    'name': 'landmines',
    'pattern': '.',
    'action': [
        'python',
        'src/build/landmines.py',
        '--landmine-scripts',
        'src/tools_libyuv/get_landmines.py',
        '--src-dir',
        'src',
    ],
  },
  # Android dependencies. Many are downloaded using Google Storage these days.
  # They're copied from https://cs.chromium.org/chromium/src/DEPS for all
  # such dependencies we share with Chromium.
  {
    # This downloads SDK extras and puts them in the
    # third_party/android_tools/sdk/extras directory.
    'name': 'sdkextras',
    'pattern': '.',
    # When adding a new sdk extras package to download, add the package
    # directory and zip file to .gitignore in third_party/android_tools.
    'action': ['python',
               'src/build/android/play_services/update.py',
               'download'
    ],
  },
  {
    'name': 'intellij',
    'pattern': '.',
    'action': ['python',
               'src/build/android/update_deps/update_third_party_deps.py',
               'download',
               '-b', 'chromium-intellij',
               '-l', 'third_party/intellij'
    ],
  },
  {
    'name': 'javax_inject',
    'pattern': '.',
    'action': ['python',
               'src/build/android/update_deps/update_third_party_deps.py',
               'download',
               '-b', 'chromium-javax-inject',
               '-l', 'third_party/javax_inject'
    ],
  },
  {
    'name': 'hamcrest',
    'pattern': '.',
    'action': ['python',
               'src/build/android/update_deps/update_third_party_deps.py',
               'download',
               '-b', 'chromium-hamcrest',
               '-l', 'third_party/hamcrest'
    ],
  },
  {
    'name': 'guava',
    'pattern': '.',
    'action': ['python',
               'src/build/android/update_deps/update_third_party_deps.py',
               'download',
               '-b', 'chromium-guava',
               '-l', 'third_party/guava'
    ],
  },
  {
    'name': 'android_support_test_runner',
    'pattern': '.',
    'action': ['python',
               'src/build/android/update_deps/update_third_party_deps.py',
               'download',
               '-b', 'chromium-android-support-test-runner',
               '-l', 'third_party/android_support_test_runner'
    ],
  },
  {
    'name': 'byte_buddy',
    'pattern': '.',
    'action': ['python',
               'src/build/android/update_deps/update_third_party_deps.py',
               'download',
               '-b', 'chromium-byte-buddy',
               '-l', 'third_party/byte_buddy'
    ],
  },
  {
    'name': 'espresso',
    'pattern': '.',
    'action': ['python',
               'src/build/android/update_deps/update_third_party_deps.py',
               'download',
               '-b', 'chromium-espresso',
               '-l', 'third_party/espresso'
    ],
  },
  {
    'name': 'robolectric_libs',
    'pattern': '.',
    'action': ['python',
               'src/build/android/update_deps/update_third_party_deps.py',
               'download',
               '-b', 'chromium-robolectric',
               '-l', 'third_party/robolectric'
    ],
  },
  {
    'name': 'apache_velocity',
    'pattern': '.',
    'action': ['python',
               'src/build/android/update_deps/update_third_party_deps.py',
               'download',
               '-b', 'chromium-apache-velocity',
               '-l', 'third_party/apache_velocity'
    ],
  },
  {
    'name': 'ow2_asm',
    'pattern': '.',
    'action': ['python',
               'src/build/android/update_deps/update_third_party_deps.py',
               'download',
               '-b', 'chromium-ow2-asm',
               '-l', 'third_party/ow2_asm'
    ],
  },
  {
    'name': 'desugar',
    'pattern': '.',
    'action': ['python',
               'src/build/android/update_deps/update_third_party_deps.py',
               'download',
               '-b', 'chromium-android-tools/bazel/desugar',
               '-l', 'third_party/bazel/desugar'
    ],
  },
  {
    'name': 'icu4j',
    'pattern': '.',
    'action': ['python',
               'src/build/android/update_deps/update_third_party_deps.py',
               'download',
               '-b', 'chromium-icu4j',
               '-l', 'third_party/icu4j'
    ],
  },
  {
    'name': 'accessibility_test_framework',
    'pattern': '.',
    'action': ['python',
               'src/build/android/update_deps/update_third_party_deps.py',
               'download',
               '-b', 'chromium-accessibility-test-framework',
               '-l', 'third_party/accessibility_test_framework'
    ],
  },
  {
    'name': 'bouncycastle',
    'pattern': '.',
    'action': ['python',
               'src/build/android/update_deps/update_third_party_deps.py',
               'download',
               '-b', 'chromium-bouncycastle',
               '-l', 'third_party/bouncycastle'
    ],
  },
  {
    'name': 'sqlite4java',
    'pattern': '.',
    'action': ['python',
               'src/build/android/update_deps/update_third_party_deps.py',
               'download',
               '-b', 'chromium-sqlite4java',
               '-l', 'third_party/sqlite4java'
    ],
  },
  {
    'name': 'xstream',
    'pattern': '.',
    'action': ['python',
               'src/build/android/update_deps/update_third_party_deps.py',
               'download',
               '-b', 'chromium-robolectric',
               '-l', 'third_party/xstream'
    ],
  },
  {
    'name': 'objenesis',
    'pattern': '.',
    'action': ['python',
               'src/build/android/update_deps/update_third_party_deps.py',
               'download',
               '-b', 'chromium-objenesis',
               '-l', 'third_party/objenesis'
    ],
  },
  {
    # Downloads the current stable linux sysroot to build/linux/ if needed.
    # This sysroot updates at about the same rate that the chrome build deps
    # change. This script is a no-op except for linux users who are doing
    # official chrome builds or cross compiling.
    'name': 'sysroot',
    'pattern': '.',
    'action': ['python', 'src/build/linux/sysroot_scripts/install-sysroot.py',
               '--running-as-hook'],
  },
  {
    # Update the Windows toolchain if necessary.
    'name': 'win_toolchain',
    'pattern': '.',
    'action': ['python', 'src/build/vs_toolchain.py', 'update'],
  },
  # Pull binutils for linux, enabled debug fission for faster linking /
  # debugging when used with clang on Ubuntu Precise.
  # https://code.google.com/p/chromium/issues/detail?id=352046
  {
    'name': 'binutils',
    'pattern': 'src/third_party/binutils',
    'action': [
        'python',
        'src/third_party/binutils/download.py',
    ],
  },
  {
    # Pull clang if needed or requested via GYP_DEFINES.
    # Note: On Win, this should run after win_toolchain, as it may use it.
    'name': 'clang',
    'pattern': '.',
    'action': ['python', 'src/tools/clang/scripts/update.py', '--if-needed'],
  },
  {
    # Update LASTCHANGE.
    'name': 'lastchange',
    'pattern': '.',
    'action': ['python', 'src/build/util/lastchange.py',
               '-o', 'src/build/util/LASTCHANGE'],
  },
  # Pull GN binaries.
  {
    'name': 'gn_win',
    'pattern': '.',
    'action': [ 'download_from_google_storage',
                '--no_resume',
                '--platform=win32',
                '--no_auth',
                '--bucket', 'chromium-gn',
                '-s', 'src/buildtools/win/gn.exe.sha1',
    ],
  },
  {
    'name': 'gn_mac',
    'pattern': '.',
    'action': [ 'download_from_google_storage',
                '--no_resume',
                '--platform=darwin',
                '--no_auth',
                '--bucket', 'chromium-gn',
                '-s', 'src/buildtools/mac/gn.sha1',
    ],
  },
  {
    'name': 'gn_linux64',
    'pattern': '.',
    'action': [ 'download_from_google_storage',
                '--no_resume',
                '--platform=linux*',
                '--no_auth',
                '--bucket', 'chromium-gn',
                '-s', 'src/buildtools/linux64/gn.sha1',
    ],
  },
  # Pull clang-format binaries using checked-in hashes.
  {
    'name': 'clang_format_win',
    'pattern': '.',
    'action': [ 'download_from_google_storage',
                '--no_resume',
                '--platform=win32',
                '--no_auth',
                '--bucket', 'chromium-clang-format',
                '-s', 'src/buildtools/win/clang-format.exe.sha1',
    ],
  },
  {
    'name': 'clang_format_mac',
    'pattern': '.',
    'action': [ 'download_from_google_storage',
                '--no_resume',
                '--platform=darwin',
                '--no_auth',
                '--bucket', 'chromium-clang-format',
                '-s', 'src/buildtools/mac/clang-format.sha1',
    ],
  },
  {
    'name': 'clang_format_linux',
    'pattern': '.',
    'action': [ 'download_from_google_storage',
                '--no_resume',
                '--platform=linux*',
                '--no_auth',
                '--bucket', 'chromium-clang-format',
                '-s', 'src/buildtools/linux64/clang-format.sha1',
    ],
  },
  # Pull luci-go binaries (isolate, swarming) using checked-in hashes.
  {
    'name': 'luci-go_win',
    'pattern': '.',
    'action': [ 'download_from_google_storage',
                '--no_resume',
                '--platform=win32',
                '--no_auth',
                '--bucket', 'chromium-luci',
                '-d', 'src/tools/luci-go/win64',
    ],
  },
  {
    'name': 'luci-go_mac',
    'pattern': '.',
    'action': [ 'download_from_google_storage',
                '--no_resume',
                '--platform=darwin',
                '--no_auth',
                '--bucket', 'chromium-luci',
                '-d', 'src/tools/luci-go/mac64',
    ],
  },
  {
    'name': 'luci-go_linux',
    'pattern': '.',
    'action': [ 'download_from_google_storage',
                '--no_resume',
                '--platform=linux*',
                '--no_auth',
                '--bucket', 'chromium-luci',
                '-d', 'src/tools/luci-go/linux64',
    ],
  },
]

recursedeps = [
  # buildtools provides clang_format, libc++, and libc++abi.
  'src/buildtools',
  # android_tools manages the NDK.
  'src/third_party/android_tools',
]
