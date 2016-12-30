JS Inc. [![Build Status](https://travis-ci.org/sjmulder/jsinc.svg?branch=master)](https://travis-ci.org/sjmulder/jsinc)
=======

Convert any file into a JavaScript ArrayBuffer.

```
Usage: jsinc <options> <file>

Options:
  -v           Print version number and exit.
  -m <style>   Use module style, see below. Default: global
  -p <prefix>  Text to prepend to the output
  -s <suffix>  Text to append to the output

Supported module styles:
  global    ; window['<file>'] = ...;
  amd       ; define('<file>', function() { return  ...; });
  commonjs  module.exports = ...;
  es6       export default ...;
```


Sample
------

```bash
echo 'Hello, World! What a great day to be alive' > sample.txt
jsinc sample.txt > sample.txt.js
```

The resulting JavaScript file looks like this:

```javascript

; window['sample.txt'] = new Uint32Array([
    0x6C6C6548, 0x57202C6F, 0x646C726F, 0x68572021, 0x61207461, 0x65726720,
    0x64207461, 0x74207961, 0x6562206F, 0x696C6120, 0x000A6576
]).buffer.slice(0, 43);
```

Use the file like so:

```html
<!DOCTYPE html>
<html>
    <head>
        <meta charset="utf-8">
        <title>JS Inc. test page</title>
    </head> 
    <body>
        <script src="sample.txt.js"></script>
        <script>
            var bytes = new Uint8Array(window['sample.txt']);
            var str = String.fromCharCode.apply(null, bytes);
            document.write(str);
        </script>
    </body>
</html>
```

This will output `Hello, World! What a great day to be alive`.


Advanced
--------

To tweak the output, use the `-p` (prefix) and `-s` (suffix) options. For
example:

```bash
jsinc sample.txt \
      -p 'define("sample.txt", function() { return' \
      -s '; });'
```

This yields:

```javascript
define("sample.txt", function() { return new Uint32Array([
    0x6C6C6548, 0x57202C6F, 0x646C726F, 0x68572021, 0x61207461, 0x65726720,
    0x64207461, 0x74207961, 0x6562206F, 0x696C6120, 0x000A6576
]).buffer.slice(0, 43); });
```


Building
--------

```bash
make        # build
make check  # test
make clean  # clean build products

make install    # into /usr/local/bin by default
make uninstall
```


Authors
-------

Written by Sijmen Mulder.

I, the copyright holder of this work, hereby release it into the public
domain. This applies worldwide.

In case this is not legally possible: I grant anyone the right to use this
work for any purpose, without any conditions, unless such conditions are
required by law.
