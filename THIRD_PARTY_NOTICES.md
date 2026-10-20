# Third-party code

- Creamlinux: https://github.com/anticitizn/creamlinux, based on commit `b69b818691983df30b86341be3c8680973295c6d`. MIT; see `LICENSE`. Original implementation by 20PercentRendered and contributors; maintained by anticitizn and contributors. The complete Git history is retained.
- mINI: `ext/ini.h`, copyright 2018 Danijel Durakovic (pulzed), MIT. Its copyright and license notice are retained in the header.
- spdlog: bundled headers under `spdlog/`, by Gabi Melman and contributors, MIT. License notices remain in the headers.
- fmt: bundled by spdlog, copyright Victor Zverovich and contributors. See `spdlog/fmt/bundled/fmt.license.rst`.
- Steamworks SDK headers: `ext/steam/`, copyright Valve Corporation. Existing upstream headers are retained with their original notices; they are not relicensed by this project.

The supplied patch's DLLs and the game's `libsteam_api.so` are not distributed. The EU4 profile retains the 88 DLC identifiers and names from the supplied configuration. Those identifiers do not contain the DLC assets.

## MIT notices for bundled implementation dependencies

Copyright (c) 2018 Danijel Durakovic (mINI).

Copyright (c) 2015-present Gabi Melman and spdlog contributors.

Copyright (c) 2012-present Victor Zverovich and fmt contributors.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
