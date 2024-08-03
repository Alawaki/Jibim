#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "common.h"
#include "utils.h"

typedef enum {
    COMMAND_UNKNOWN,
    COMMAND_INCOME,
    COMMAND_EXPENSE,
    COMMAND_SUMMARY
} Command;



typedef struct {
    int expense_flag;
    int income_flag;
    int tag_flag;
    Amount income_value;
    Amount expense_value;
    Tag tag_value;
} Option;




void print_usage() {
    printf("Usage:\n");
    printf(" - To record expense: jibim -e [amount] -t [tag]\n");
    printf(" - To record income: jibim -i [amount] -t [tag]\n");
}


int get_args(int argc, char** argv, Option* o) {
    int opt;
    o->income_flag = 0;
    o->expense_flag = 0;
    o->tag_flag = 0;
    do {
        opt = getopt(argc, argv, "i:t:e:s:");
        switch(opt) {
            case 'i':
                o->income_flag = 1;
                strncpy(o->income_value, optarg, MAX_AMOUNT_SIZE);
                break;
            case 't':
                o->tag_flag = 1;
                strncpy(o->tag_value, optarg, MAX_TAG_SIZE);
                break;
            case 'e':
                o->expense_flag = 1;
                strncpy(o->expense_value, optarg, 10);
                break;
            case -1:
                break;
            default:
                
                return EINVAL;
        }
    } while(opt != -1);
    return SUCCESS;
}

Command get_command(Option o){
    if (o.income_flag == 1 && o.expense_flag == 1){
        printf("Can not have income and expense together!\n");
        return COMMAND_UNKNOWN;
    }else if((o.income_flag == 1 || o.expense_flag == 1) && o.tag_flag != 1){
        printf("PLease specify a tag using -t!\n");
        return COMMAND_UNKNOWN;
    }else if (o.income_flag){
        return COMMAND_INCOME;
    }else if(o.expense_flag){
        return COMMAND_EXPENSE;
    }
    
    return COMMAND_SUMMARY;
}

int append(char* amount_value, char* tag_value, double mul){
    Date date;
    double amount;  

    get_date(&date);



    if (convert_str_to_double(amount_value, &amount) != SUCCESS){
        return ERROR;
    }
    
    if(amount <= 0.0){
        printf("Amount[%.2lf] can not be less than zero!\n", amount);
        return ERROR;
    }

    FILE* f = fopen("journal.tsv", "a");
    if ( f != NULL){
        fprintf(f, "%s\t%.2lf\t%s\n", date, mul * amount, tag_value);
        fclose(f);
    }

    return SUCCESS;
}

int record_expense(Option o){
    return append(o.expense_value, o.tag_value, -1.0);
}

int record_income(Option o){
    return append(o.income_value, o.tag_value, 1.0);
}

int print_summary(Option o){
    Journal row;
    double sum = 0;
    int max_rows_to_print = 5;

    FILE* f = fopen("journal.tsv", "r");
    if ( f == NULL){
        printf("Nothing to summrize!\n");
        return ERROR;
    }
    
    while(fscanf(f, "%s\t%lf\t%s", row.date, &row.amount, row.tag) == 3){
        sum = sum + row.amount;
        max_rows_to_print--;
        if(max_rows_to_print >= 0){
            debug_journal(row);       
        } 
    }
    fclose(f);
    printf("--------------------\n");
    printf("Current:\n\t%.2lf$\n", sum);
    return SUCCESS;
}

int main(int argc, char** argv){
    Option o;
    Journal j;

    int result;
    result = get_args(argc, argv, &o);
    if (result != 0){
        print_usage();
        return result;
    }
    Command cmd = get_command(o);
    switch(cmd){
        case COMMAND_UNKNOWN:
            print_usage();
            return ERROR;
        case COMMAND_EXPENSE:
            return record_expense(o);
        case COMMAND_INCOME:
            return record_income(o);
        case COMMAND_SUMMARY:
            return print_summary(o);
    } 
    
    return SUCCESS;
}