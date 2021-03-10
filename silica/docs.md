# Silica
The best programming language out there!

The syntax is made to be simple, easy to read and understand for non-Silica programmers


##### The let statement
```Silica
let <identifier> = <value>
let <type> <identifier> = <value>
```
The let statement declares a variable which cannot be changed, and therefore requires a value. You can optionally specify the type before the name.

##### The const statement
```Silica
const <identifier> = <value>
const <type> <identifier> = <value>
```
The const statement works a lot like the let statement, but requires that the value is known at compile time.

##### The var statement
```Silica
var <identifier> = <value>
var <type> <identifier> = <value>
var <type> <identifier>
```
The var statement allows you to declare a variable which can be modified, which means that you can declare it without a value.

##### The func declaration
```
func <return type> <identifier>(<arguments>)<whitespace><expression>
<arguments> => comma seperated list of <argument>s
<argument> => <type> <name> <label>
```
Wait, how are supposed to do anything meaningful with just one expression!
That's where the block expression comes in:

##### The block expression
```
:<todo, block arguments><newline><newline seperated statements>
```


##### Types
`Float16`: A 16 bit float (IEEE 754 binary32)

`Float32`: A 32 bit float (IEEE 754 binary32)

`Float64`: A 64 bit float (IEEE 754 binary64), recommended for most cases.

`Int`: A pointer sized signed integer type

`UInt`: A pointer sized unsigned integer type

`Int<N>`: An N bit signed integer type

`UInt<N>`: An N bit unsigned integer type

e.g `UInt8` and `Int32`













```
```