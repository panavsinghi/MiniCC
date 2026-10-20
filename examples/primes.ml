let n = 2;
while (n <= 50) {
    let d = 2;
    let prime = 1;
    while (d * d <= n && prime) {
        if (n % d == 0) { prime = 0; }
        d = d + 1;
    }
    if (prime) { print n; }
    n = n + 1;
}
