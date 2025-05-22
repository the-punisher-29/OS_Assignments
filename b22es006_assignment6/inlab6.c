#include <stdio.h>
#include <pthread.h>

typedef struct {
    int num1;
    int num2;
    int result;
} SumArgs;

// Thread function to compute the sum
void* compute_sum(void* arg) {
    SumArgs* args = (SumArgs*)arg;
    args->result = args->num1 + args->num2;
    pthread_exit(NULL);
}

int main() {
    pthread_t thread;
    SumArgs args;
    int a,b;
    printf("Enter two numbers: ");
    scanf("%d %d", &a, &b);//taking input from the user and storing it in a and b(main thread)
    args.num1 =a;
    args.num2 =b;
    pthread_create(&thread, NULL, compute_sum, &args);//creating a thread(child) that will compute the sum of the numbers and passing the argument from main thread
    pthread_join(thread, NULL);//this ensures the parent process waits for the thread to finish
    printf("The sum is: %d\n", args.result);
    return 0;
}





