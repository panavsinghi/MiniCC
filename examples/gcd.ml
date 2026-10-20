let a = 1071;
let b = 462;
while (b != 0) {
    let t = b;
    b = a % b;
    a = t;
}
print a;
