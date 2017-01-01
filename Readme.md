JS Inc. [![Build Status](https://travis-ci.org/sjmulder/jsinc.svg?branch=master)](https://travis-ci.org/sjmulder/jsinc)
=======

Convert any file into a JavaScript ArrayBuffer.

```
Usage: jsinc <options> <file>

Options:
  -v           Print version number and exit.
  -m <style>   Use module style, see below. Default: global
  -f <format>  Use output format, see below. Default: arraybuf
  -p <prefix>  Text to prepend to the output
  -s <suffix>  Text to append to the output
  -c           Do not output a generator comment

Supported module styles:
  global    ; window['<file>'] = ...;
  amd       ; define('<file>', function() { return  ...; });
  commonjs  module.exports = ...;
  es6       export default ...;
  none      ...

Supported output formats:
  arraybuf  ArrayBuffer constructerd using a TypedArray
  array     Array of byte values
  string    A string (escaped where needed)
  base64    A base 64-encoded string
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

By default, an ArrayBuffer object is generated. This can be changed using the
`-f` (output format) option. For example:

```bash
jsinc -f array  sample.txt
jsinc -f string sample.txt
jsinc -f base64 sample.txt
```

```javascript
; window['sample.txt'] = [
     72,101,108,108,111, 44, 32, 87,111,114,108,100, 33, 32, 87,104, 97,
    116, 32, 97, 32,103,114,101, 97,116, 32,100, 97,121, 32,116,111, 32,
     98,101, 32, 97,108,105,118,101, 10
];
```

```javascript
; window['fixtures/short.txt'] = 'Hello, World! What a great day to be alive\
\n';
```

```javascript
; window['sample.txt'] = 'SGVsbG8sIFdvcmxkISBXaGF0IGEgZ3JlYXQgZGF5IHRvIGJlIGF\
saXZlCg==';
```

The output can also be tweaked using the `-p` (prefix) and `-s` (suffix)
options. For example:

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

From a source distribution:

```bash
make        # build
make check  # test
make dist   # creates a .zip of the binary
make clean  # clean build products

make install    # into /usr/local/bin by default
make uninstall
```

For Windows with MSYS:

```bash
make -f Makefile.msys
make -f Makefile.msys check
make -f Makefile.msys dist
make -f Makefile.msys clean
```

See Makefile and Makefile.msys for more options, or .travis.yml for some
examples on cross compiling and such.


Authors
-------

Written by Sijmen Mulder.

I, the copyright holder of this work, hereby release it into the public
domain. This applies worldwide.

In case this is not legally possible: I grant anyone the right to use this
work for any purpose, without any conditions, unless such conditions are
required by law.
