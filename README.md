# markc

Garbage collection for C! Isn't that cool?

## How to use

Initiate the gc with `void markc_start(mark *gc, var bottom_of_stack)`. Like this:

```c
#include "markc.h"

mark gc = {};

int main(int argc, string *argv) {
    markc_start(&gc, &argc); // "argc" is the first variable and therefore the bottom of the stack.

    // var() is __auto_type. it infers the type from its initializer.
    // var is `void*`. there's a difference!
    var() x = new(int, 5);
    var u = new(int, 7);

    // now we clean memory. typically nothing should get deleted here.
    markc_collect(&gc);

    markc_stop(&gc); // WARNING: This clears ALL memory that has been allocated with markc.

    return 0;
}
```

MarkC also throws you helpful error messages when malloc or realloc fail after 10 tries. It also doesn't automatically collect garbage, Which you'd need to enable with:

```c
#define MARKC_AUTO_COLLECT
#include "markc.h"
```

MarkC also doesn't notify you if you try to free a pointer twice. It only frees pointers it owns:

```c
var x = new(int, 8);
markc_free(&gc, x); // this is fine
markc_free(&gc, x); // this is still fine!
```

Disable this behavior with:

```c
#define __BRAVE__
#include "markc.h"
```
