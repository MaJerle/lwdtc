# Lightweight Date, Time & Cron

LwDTC is a lightweight, platform independent library for date, time and cron utility management.

Main motivation comes from the necessity of simple cron for my own personal home automation project, with `1` second granularity.

[Open documentation](https://docs.majerle.eu/projects/lwdtc/)

## Features

* Written in C (C11), compatible with `stdint.h` data types
* Platform independent, easy to use
* Support for date, time and cron utilities
* Ultra-lightweight cron library for embedded systems
* Cron supports numbers only, no string dates/months, quicker parsing
* Support for the `struct tm` data structure from `time.h` for time operations
* Date and time range support with CRON syntax
* Wrap-around range syntax, e.g. max-min, to cross the field boundary
* Parse multiple cron expressions in a single call, with failing index reported
* Combine multiple cron contexts with AND or OR match semantics
* Calculate the next matching run time for a cron expression
* User friendly MIT license

## Contribute

Fresh contributions are always welcome. Simple instructions to proceed:

1. Fork Github repository
2. Follow [C style & coding rules](https://github.com/MaJerle/c-code-style) and use `clang-format` to format the code
3. Create a pull request to `develop` branch with new features or bug fixes

Alternatively you may:

1. Report a bug
2. Ask for a feature request
