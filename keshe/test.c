int addi(int a, int b);
int fact1(int a);
int fact2(int a);

int addi (int a, int b){
    return a + b;
}

int fact1 (int a) {
    int i =1, sum = 1;
    while (i <= a) {
        sum = sum* i;
        i = i+ 1;
    }
    return i;
}

int fact2 (int a) {
    int sum = 0;
    for (int i = 0; i< a; i = i+1)
        sum = sum*i;
    return sum;
}

int isodd(int a) {
    if (a %2 == 1) {
        return 1;
    }
    return 0;
}

int main () 

{
    int a =7, b = 04;
    int c = 0x23;
    c = c%3- a *add(a,b)+fact(a)+0x111;
    if(isodd(c)) return 1;
    else return 0;
}