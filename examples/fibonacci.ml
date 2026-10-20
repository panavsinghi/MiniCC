let a = 0;
let b = 1;
let i = 0;
while (i < 20) {
    print a;
    let next = a + b;
    a = b;
    b = next;
    i = i + 1;
}
