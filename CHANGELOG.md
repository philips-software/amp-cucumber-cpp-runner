# Changelog

## 1.0.0 (2024-07-22)


### ⚠ BREAKING CHANGES

* added input validation, report file and folder outpt options and added the 'run' sub-command ([#57](https://github.com/philips-software/amp-cucumber-cpp-runner/issues/57))

### Features

* Add custom report handler ([#32](https://github.com/philips-software/amp-cucumber-cpp-runner/issues/32)) ([051764f](https://github.com/philips-software/amp-cucumber-cpp-runner/commit/051764f7b8459e61329272dbbad54e73b0f00293))
* Added custom fixture option for steps ([#4](https://github.com/philips-software/amp-cucumber-cpp-runner/issues/4)) ([54099e3](https://github.com/philips-software/amp-cucumber-cpp-runner/commit/54099e3673b430b7fa90ced531a7a201570f544d))
* Added input validation, report file and folder outpt options and added the 'run' sub-command ([#57](https://github.com/philips-software/amp-cucumber-cpp-runner/issues/57)) ([c464620](https://github.com/philips-software/amp-cucumber-cpp-runner/commit/c46462034574001fab1aee4479f55da1ae384f0a))
* Added InternalError with SourceLocation ([#35](https://github.com/philips-software/amp-cucumber-cpp-runner/issues/35)) ([830ff6d](https://github.com/philips-software/amp-cucumber-cpp-runner/commit/830ff6d04635ee5d99fc1846a0cadf4aceeb572d))
* Added release please workflow ([#9](https://github.com/philips-software/amp-cucumber-cpp-runner/issues/9)) ([103b282](https://github.com/philips-software/amp-cucumber-cpp-runner/commit/103b2825e34579e79215b917c4387e41b59d3de5))
* Added SetUp and TearDown functions to step implementations ([#37](https://github.com/philips-software/amp-cucumber-cpp-runner/issues/37)) ([b18e53d](https://github.com/philips-software/amp-cucumber-cpp-runner/commit/b18e53d13d80484d0ad4919cac7afd9c75fdfb3d))
* Added support for parsing step function arguments that are cv-ref qualified ([#18](https://github.com/philips-software/amp-cucumber-cpp-runner/issues/18)) ([a1b4a9a](https://github.com/philips-software/amp-cucumber-cpp-runner/commit/a1b4a9a23e10002cf51b6bd366faeffe91ce2aa5))
* Added support for passing in a directory with feature files ([03af907](https://github.com/philips-software/amp-cucumber-cpp-runner/commit/03af907dc65b7eca4840930a9c920570ad34d240))
* Added support for passing in a directory with feature files ([#31](https://github.com/philips-software/amp-cucumber-cpp-runner/issues/31)) ([fd8eae6](https://github.com/philips-software/amp-cucumber-cpp-runner/commit/fd8eae6179d844d73a758ec6a080dfc4a7138449))
* Added the ability to call other steps from any step ([#5](https://github.com/philips-software/amp-cucumber-cpp-runner/issues/5)) ([e9eb924](https://github.com/philips-software/amp-cucumber-cpp-runner/commit/e9eb924abec73a6ea09b07c476376da96c89ca5a))
* An undefined test result will not set the global status to undefined if it was previously a success status ([#55](https://github.com/philips-software/amp-cucumber-cpp-runner/issues/55)) ([68b97f8](https://github.com/philips-software/amp-cucumber-cpp-runner/commit/68b97f8c308954598c3c2f5feff183c1f2fc38b7))
* Ci.yml now builds on push to main instead of scheduled nightly ([#2](https://github.com/philips-software/amp-cucumber-cpp-runner/issues/2)) ([f6ce586](https://github.com/philips-software/amp-cucumber-cpp-runner/commit/f6ce58645cbd6ab2533ec9d04b43bb749fd48068))
* Context::Contains is now a const function ([#6](https://github.com/philips-software/amp-cucumber-cpp-runner/issues/6)) ([9c2eb07](https://github.com/philips-software/amp-cucumber-cpp-runner/commit/9c2eb073525906660a6913f7fbc367414624385e))
* Context::Get/GetShared now throws KeyNotFound instead of std::out_of_range ([#7](https://github.com/philips-software/amp-cucumber-cpp-runner/issues/7)) ([10b83eb](https://github.com/philips-software/amp-cucumber-cpp-runner/commit/10b83eb4d9a625f05131baefb9d565e621727ea9))
* **deps:** Bump philips-software/amp-devcontainer from v4.6.0 to v4.7.0 in /.devcontainer ([#50](https://github.com/philips-software/amp-cucumber-cpp-runner/issues/50)) ([71987b2](https://github.com/philips-software/amp-cucumber-cpp-runner/commit/71987b290cda5e8c009771a15baf8ec28d7db778))
* Initial commit ([5d0973b](https://github.com/philips-software/amp-cucumber-cpp-runner/commit/5d0973bb449a0a1d3cb6b0d7ff677a4ef620fdeb))
* Provide value like access to context and table for steps and hooks ([#3](https://github.com/philips-software/amp-cucumber-cpp-runner/issues/3)) ([60f598c](https://github.com/philips-software/amp-cucumber-cpp-runner/commit/60f598c9acf6c131e090c5fead021f72f8a2fa22))
* Update to official gherkin/messages cpp ([#22](https://github.com/philips-software/amp-cucumber-cpp-runner/issues/22)) ([8d0a64a](https://github.com/philips-software/amp-cucumber-cpp-runner/commit/8d0a64a8c88d025441c74185332bc8faa57442c0))
* Use cliutils-cli11 argument parser instead of a homebrew implementation ([#56](https://github.com/philips-software/amp-cucumber-cpp-runner/issues/56)) ([2a44af4](https://github.com/philips-software/amp-cucumber-cpp-runner/commit/2a44af48b829f884fd97db4d2a16610663362597))


### Bug Fixes

* Fixed --Xapp argument splitting for clang-cl/winsdk ([#8](https://github.com/philips-software/amp-cucumber-cpp-runner/issues/8)) ([71c8273](https://github.com/philips-software/amp-cucumber-cpp-runner/commit/71c8273fb61b503380e12f014d5da920e2ce19d2))
* Tag expression parsing for scenarios with no tags ([#65](https://github.com/philips-software/amp-cucumber-cpp-runner/issues/65)) ([604b85d](https://github.com/philips-software/amp-cucumber-cpp-runner/commit/604b85d195c21ee0ac40971550ee7b3ba505b43f))
* Various fixes ([#1](https://github.com/philips-software/amp-cucumber-cpp-runner/issues/1)) ([2838230](https://github.com/philips-software/amp-cucumber-cpp-runner/commit/28382308fcb78f20983960dac2533e533c2c9ecc))
