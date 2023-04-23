# Getting Started with `cppproptest`

## Running your first property-based test

`cppproptest` requires [CMake](https://cmake.org) for build tool.
You can examine the requirements with:

```Shell
# at cppproptest root directory
$ cmake . -BBUILD
$ cd BUILD && make && ./test_proptest
```

You can edit your project's CMakeLists.txt to include the library `proptest`:

```CMake
ADD_SUBDIRECTORY(<path_to_cppproptest_root>)

```

And then add this library to `TARGET_LINK_LIBRARIES` section.

```
TARGET_LINK_LIBRARIES( ...
    ...
    proptest
    ...
)
```

Here's an example using [Googletest](https://github.com/google/googletest):

```cpp
// ...
#include "proptest/proptest.hpp"

using namespace proptest;

TEST(AudioCodec, EncoderDecoder)
{
    // ASSERT_FOR_ALL() is shorthand for ASSERT_TRUE(forAll(...))
    ASSERT_FOR_ALL([](SoundData soundData) {
        auto encoded = MyAudioCodec::encode(soundData);
        auto decoded = MyAudioCodec::decode(encoded);
        PROP_ASSERT_EQ(decoded, soundData);
    });
}
```

Note that `ASSERT_FOR_ALL` is a simple macro wrapping an `ASSERT_TRUE` google test macro around the `proptest::forAll` function. You can find more information in [Using Assertion](#using-assertions) section.

You can continue to [What You Can Do with `cppproptest`](Property.md)
