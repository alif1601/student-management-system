#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#define MAX_STUDENTS 1000
#define MAX_USERS 1000
#define MAX_ADMINS 100
#define MAX_COURSES 20

#define STUDENT_FILE "students.txt"
#define USER_FILE "user_accounts.txt"
#define ADMIN_FILE "admin_accounts.txt"
#define USER_LOGIN_FILE "user_login_history.txt"
#define ADMIN_LOGIN_FILE "admin_login_history.txt"

#ifdef _WIN32
#define CLEAR_COMMAND "cls"
#else
#define CLEAR_COMMAND "clear"
#endif

typedef struct
{
    char code[21];
    char name[41];
    float credit;
    float mark;
    char grade[3];
    float grade_point;
} Course;

typedef struct
{
    char student_id[11];
    char name[50];
    char department[50];
    char semester[20];
    char section[10];
    char phone[20];
    char email[50];
    char dob[20];
    char gender[15];
    char guardian_name[50];
    char guardian_phone[20];
    char address[100];
    int admission_year;

    int course_count;
    Course courses[MAX_COURSES];

    float total_credits;
    float sgpa;
    float attendance;
    char fee_status[20];
} Student;

typedef struct
{
    char student_id[11];
    char password[31];
    char created_at[32];
} UserAccount;

typedef struct
{
    char username[31];
    char password[31];
    char created_by[31];
    char created_at[32];
} AdminAccount;

static void clear_screen(void)
{
    system(CLEAR_COMMAND);
}

static void read_line(char *b, size_t s)
{
    if (fgets(b, (int)s, stdin))
        b[strcspn(b, "\n")] = '\0';
    else if (s)
        b[0] = '\0';
}

static void wait_for_enter(void)
{
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF) {}
}

static void pause_screen(void)
{
    printf("\nPress Enter to continue...");
    fflush(stdout);
    wait_for_enter();
}

static void print_spaces(int n)
{
    int i;
    for (i = 0; i < n; i++) printf(" ");
}

static int get_center_padding(int content_width)
{
    int console_width = 100;
    int pad = (console_width - content_width) / 2;
    if (pad < 0) pad = 0;
    return pad;
}

static void center_line(const char *text)
{
    int pad = get_center_padding((int)strlen(text));
    print_spaces(pad);
    printf("%s\n", text);
}

static void print_menu_line(const char *text, int block_width)
{
    int pad = get_center_padding(block_width);
    print_spaces(pad);
    printf("%s\n", text);
}

static void centered_title(const char *text)
{
    int console_width = 100;
    int box_width = 68;
    int inner_width = box_width - 2;
    int len = (int)strlen(text);
    int left_padding = (console_width - box_width) / 2;
    int left_text = (inner_width - len) / 2;
    int right_text;

    if (left_text < 0) left_text = 0;
    right_text = inner_width - left_text - len;
    if (right_text < 0) right_text = 0;

    print_spaces(left_padding);
    printf("+------------------------------------------------------------------+\n");

    print_spaces(left_padding);
    printf("|                                                                  |\n");

    print_spaces(left_padding);
    printf("|");
    print_spaces(left_text);
    printf("%s", text);
    print_spaces(right_text);
    printf("|\n");

    print_spaces(left_padding);
    printf("|                                                                  |\n");

    print_spaces(left_padding);
    printf("+------------------------------------------------------------------+\n");
}

static int get_int_input(const char *prompt, int min, int max)
{
    char line[100], *end;
    long value;

    for (;;)
    {
        printf("%s", prompt);
        read_line(line, sizeof(line));

        if (!line[0])
        {
            puts("Invalid input. Please try again.");
            continue;
        }

        value = strtol(line, &end, 10);
        if (*end)
        {
            puts("Invalid input. Please enter a whole number.");
            continue;
        }

        if (value < min || value > max)
        {
            printf("Please enter a value between %d and %d.\n", min, max);
            continue;
        }

        return (int)value;
    }
}

static float get_float_input(const char *prompt, float min, float max)
{
    char line[100], extra;
    float value;

    for (;;)
    {
        printf("%s", prompt);
        read_line(line, sizeof(line));

        if (!line[0])
        {
            puts("Invalid input. Please try again.");
            continue;
        }

        if (sscanf(line, "%f %c", &value, &extra) != 1)
        {
            puts("Invalid input. Please enter a valid number.");
            continue;
        }

        if (value < min || value > max)
        {
            printf("Please enter a value between %.2f and %.2f.\n", min, max);
            continue;
        }

        return value;
    }
}

static void get_nonempty_input(const char *prompt, char *buffer, size_t size)
{
    for (;;)
    {
        printf("%s", prompt);
        read_line(buffer, size);

        if (!buffer[0])
        {
            puts("Input cannot be empty. Please try again.");
            continue;
        }

        if (strchr(buffer, '|'))
        {
            puts("Character '|' is not allowed. Please try again.");
            continue;
        }

        return;
    }
}

static int is_valid_student_id(const char *id)
{
    size_t i, len = strlen(id);

    if (len < 4 || len > 10) return 0;

    for (i = 0; i < len; i++)
        if (!isdigit((unsigned char)id[i])) return 0;

    return 1;
}

static int is_valid_username(const char *name)
{
    size_t i, len = strlen(name);

    if (len < 3 || len > 30) return 0;

    for (i = 0; i < len; i++)
        if (!(isalnum((unsigned char)name[i]) || name[i] == '_' || name[i] == '.'))
            return 0;

    return 1;
}

static void get_now(char *buffer, size_t size)
{
    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    if (t) strftime(buffer, size, "%Y-%m-%d %H:%M:%S", t);
    else if (size) buffer[0] = '\0';
}

static float mark_to_grade_point(float mark)
{
    if (mark >= 80) return 4.0f;
    if (mark >= 75) return 3.75f;
    if (mark >= 70) return 3.5f;
    if (mark >= 65) return 3.25f;
    if (mark >= 60) return 3.0f;
    if (mark >= 55) return 2.75f;
    if (mark >= 50) return 2.5f;
    if (mark >= 45) return 2.25f;
    if (mark >= 40) return 2.0f;
    return 0.0f;
}

static const char *mark_to_grade(float mark)
{
    if (mark >= 80) return "A+";
    if (mark >= 75) return "A";
    if (mark >= 70) return "A-";
    if (mark >= 65) return "B+";
    if (mark >= 60) return "B";
    if (mark >= 55) return "B-";
    if (mark >= 50) return "C+";
    if (mark >= 45) return "C";
    if (mark >= 40) return "D";
    return "F";
}

static void append_login_history(const char *filename, const char *role, const char *name, const char *status)
{
    FILE *fp = fopen(filename, "a");
    char now[32];

    if (!fp) return;

    get_now(now, sizeof(now));
    fprintf(fp, "| %-19s | %-8s | %-28s | %-16s |\n", now, role, name, status);
    fclose(fp);
}

static int find_student_index(const Student s[], int count, const char *id)
{
    int i;
    for (i = 0; i < count; i++)
        if (strcmp(s[i].student_id, id) == 0) return i;
    return -1;
}

static int find_user_index(const UserAccount u[], int count, const char *id)
{
    int i;
    for (i = 0; i < count; i++)
        if (strcmp(u[i].student_id, id) == 0) return i;
    return -1;
}

static int find_admin_index(const AdminAccount a[], int count, const char *username)
{
    int i;
    for (i = 0; i < count; i++)
        if (strcmp(a[i].username, username) == 0) return i;
    return -1;
}

static int load_students(Student s[], int max)
{
    FILE *fp = fopen(STUDENT_FILE, "r");
    char line[512];
    int count = 0;

    if (!fp) return 0;

    while (count < max && fgets(line, sizeof(line), fp))
    {
        char *tok;
        int i;

        if (strncmp(line, "STUDENT|", 8) != 0) continue;

        line[strcspn(line, "\n")] = '\0';

        strtok(line, "|");

        tok = strtok(NULL, "|");
        strncpy(s[count].student_id, tok ? tok : "", 10);
        s[count].student_id[10] = '\0';

        tok = strtok(NULL, "|");
        strncpy(s[count].name, tok ? tok : "", 49);
        s[count].name[49] = '\0';

        tok = strtok(NULL, "|");
        strncpy(s[count].department, tok ? tok : "", 49);
        s[count].department[49] = '\0';

        tok = strtok(NULL, "|");
        strncpy(s[count].semester, tok ? tok : "", 19);
        s[count].semester[19] = '\0';

        tok = strtok(NULL, "|");
        strncpy(s[count].section, tok ? tok : "", 9);
        s[count].section[9] = '\0';

        tok = strtok(NULL, "|");
        strncpy(s[count].phone, tok ? tok : "", 19);
        s[count].phone[19] = '\0';

        tok = strtok(NULL, "|");
        strncpy(s[count].email, tok ? tok : "", 49);
        s[count].email[49] = '\0';

        tok = strtok(NULL, "|");
        strncpy(s[count].dob, tok ? tok : "", 19);
        s[count].dob[19] = '\0';

        tok = strtok(NULL, "|");
        strncpy(s[count].gender, tok ? tok : "", 14);
        s[count].gender[14] = '\0';

        tok = strtok(NULL, "|");
        strncpy(s[count].guardian_name, tok ? tok : "", 49);
        s[count].guardian_name[49] = '\0';

        tok = strtok(NULL, "|");
        strncpy(s[count].guardian_phone, tok ? tok : "", 19);
        s[count].guardian_phone[19] = '\0';

        tok = strtok(NULL, "|");
        strncpy(s[count].address, tok ? tok : "", 99);
        s[count].address[99] = '\0';

        tok = strtok(NULL, "|");
        s[count].admission_year = tok ? atoi(tok) : 0;

        tok = strtok(NULL, "|");
        s[count].course_count = tok ? atoi(tok) : 0;

        tok = strtok(NULL, "|");
        s[count].total_credits = tok ? (float)atof(tok) : 0.0f;

        tok = strtok(NULL, "|");
        s[count].sgpa = tok ? (float)atof(tok) : 0.0f;

        tok = strtok(NULL, "|");
        s[count].attendance = tok ? (float)atof(tok) : 0.0f;

        tok = strtok(NULL, "|");
        strncpy(s[count].fee_status, tok ? tok : "", 19);
        s[count].fee_status[19] = '\0';

        if (s[count].course_count < 0 || s[count].course_count > MAX_COURSES)
            s[count].course_count = 0;

        for (i = 0; i < s[count].course_count; i++)
        {
            char *p;

            if (!fgets(line, sizeof(line), fp)) break;
            line[strcspn(line, "\n")] = '\0';

            if (strncmp(line, "COURSE|", 7) != 0) break;

            strtok(line, "|");

            p = strtok(NULL, "|");
            strncpy(s[count].courses[i].code, p ? p : "", 20);
            s[count].courses[i].code[20] = '\0';

            p = strtok(NULL, "|");
            strncpy(s[count].courses[i].name, p ? p : "", 40);
            s[count].courses[i].name[40] = '\0';

            p = strtok(NULL, "|");
            s[count].courses[i].credit = p ? (float)atof(p) : 0.0f;

            p = strtok(NULL, "|");
            s[count].courses[i].mark = p ? (float)atof(p) : 0.0f;

            p = strtok(NULL, "|");
            strncpy(s[count].courses[i].grade, p ? p : "", 2);
            s[count].courses[i].grade[2] = '\0';

            p = strtok(NULL, "|");
            s[count].courses[i].grade_point = p ? (float)atof(p) : 0.0f;
        }

        while (fgets(line, sizeof(line), fp))
            if (strncmp(line, "ENDSTUDENT", 10) == 0) break;

        count++;
    }

    fclose(fp);
    return count;
}

static int save_students(const Student s[], int count)
{
    FILE *fp = fopen(STUDENT_FILE, "w");
    int i, j;

    if (!fp) return 0;

    fprintf(fp, "# Final University Project Student Data File\n");
    fprintf(fp, "# STUDENT|id|name|department|semester|section|phone|email|dob|gender|guardian_name|guardian_phone|address|admission_year|course_count|total_credits|sgpa|attendance|fee_status\n");
    fprintf(fp, "# COURSE|code|name|credit|mark|grade|grade_point\n\n");

    for (i = 0; i < count; i++)
    {
        fprintf(fp,
                "STUDENT|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%d|%d|%.2f|%.2f|%.2f|%s\n",
                s[i].student_id,
                s[i].name,
                s[i].department,
                s[i].semester,
                s[i].section,
                s[i].phone,
                s[i].email,
                s[i].dob,
                s[i].gender,
                s[i].guardian_name,
                s[i].guardian_phone,
                s[i].address,
                s[i].admission_year,
                s[i].course_count,
                s[i].total_credits,
                s[i].sgpa,
                s[i].attendance,
                s[i].fee_status);

        for (j = 0; j < s[i].course_count; j++)
        {
            fprintf(fp, "COURSE|%s|%s|%.2f|%.2f|%s|%.2f\n",
                    s[i].courses[j].code,
                    s[i].courses[j].name,
                    s[i].courses[j].credit,
                    s[i].courses[j].mark,
                    s[i].courses[j].grade,
                    s[i].courses[j].grade_point);
        }

        fprintf(fp, "ENDSTUDENT\n\n");
    }

    fclose(fp);
    return 1;
}

static int load_users(UserAccount u[], int max)
{
    FILE *fp = fopen(USER_FILE, "r");
    char line[256];
    int count = 0;

    if (!fp) return 0;

    while (count < max && fgets(line, sizeof(line), fp))
    {
        char *p;

        if (strncmp(line, "USER|", 5) != 0) continue;
        line[strcspn(line, "\n")] = '\0';

        strtok(line, "|");

        p = strtok(NULL, "|");
        strncpy(u[count].student_id, p ? p : "", 10);
        u[count].student_id[10] = '\0';

        p = strtok(NULL, "|");
        strncpy(u[count].password, p ? p : "", 30);
        u[count].password[30] = '\0';

        p = strtok(NULL, "|");
        strncpy(u[count].created_at, p ? p : "", 31);
        u[count].created_at[31] = '\0';

        count++;
    }

    fclose(fp);
    return count;
}

static int save_users(const UserAccount u[], int count)
{
    FILE *fp = fopen(USER_FILE, "w");
    int i;

    if (!fp) return 0;

    fprintf(fp, "# User Accounts\n# USER|student_id|password|created_at\n\n");
    for (i = 0; i < count; i++)
        fprintf(fp, "USER|%s|%s|%s\n", u[i].student_id, u[i].password, u[i].created_at);

    fclose(fp);
    return 1;
}

static int load_admins(AdminAccount a[], int max)
{
    FILE *fp = fopen(ADMIN_FILE, "r");
    char line[256];
    int count = 0;

    if (!fp) return 0;

    while (count < max && fgets(line, sizeof(line), fp))
    {
        char *p;

        if (strncmp(line, "ADMIN|", 6) != 0) continue;
        line[strcspn(line, "\n")] = '\0';

        strtok(line, "|");

        p = strtok(NULL, "|");
        strncpy(a[count].username, p ? p : "", 30);
        a[count].username[30] = '\0';

        p = strtok(NULL, "|");
        strncpy(a[count].password, p ? p : "", 30);
        a[count].password[30] = '\0';

        p = strtok(NULL, "|");
        strncpy(a[count].created_by, p ? p : "", 30);
        a[count].created_by[30] = '\0';

        p = strtok(NULL, "|");
        strncpy(a[count].created_at, p ? p : "", 31);
        a[count].created_at[31] = '\0';

        count++;
    }

    fclose(fp);
    return count;
}

static int save_admins(const AdminAccount a[], int count)
{
    FILE *fp = fopen(ADMIN_FILE, "w");
    int i;

    if (!fp) return 0;

    fprintf(fp, "# Admin Accounts\n# ADMIN|username|password|created_by|created_at\n\n");
    for (i = 0; i < count; i++)
        fprintf(fp, "ADMIN|%s|%s|%s|%s\n", a[i].username, a[i].password, a[i].created_by, a[i].created_at);

    fclose(fp);
    return 1;
}

static void ensure_default_admin(void)
{
    AdminAccount a[MAX_ADMINS];
    int count = load_admins(a, MAX_ADMINS);

    if (count == 0)
    {
        strcpy(a[0].username, "alif");
        strcpy(a[0].password, "1122");
        strcpy(a[0].created_by, "system");
        get_now(a[0].created_at, sizeof(a[0].created_at));
        save_admins(a, 1);
    }
}

static void print_student_summary_header(void)
{
    printf("+----+------------+----------------------+-------------+-------------+\n");
    printf("| SL | Student ID | Name                 | Department  | SGPA        |\n");
    printf("+----+------------+----------------------+-------------+-------------+\n");
}

static void print_student_summary_row(const Student *s, int serial)
{
    printf("| %-2d | %-10s | %-20s | %-11s | %-11.2f |\n",
           serial, s->student_id, s->name, s->department, s->sgpa);
}

static void print_student_full_details(const Student *s, int serial)
{
    int i;

    printf("\n+================================================================================+\n");
    printf("| Student Serial   : %-60d |\n", serial);
    printf("| Student ID       : %-60s |\n", s->student_id);
    printf("| Name             : %-60s |\n", s->name);
    printf("| Department       : %-60s |\n", s->department);
    printf("| Semester         : %-60s |\n", s->semester);
    printf("| Section          : %-60s |\n", s->section);
    printf("| Phone            : %-60s |\n", s->phone);
    printf("| Email            : %-60s |\n", s->email);
    printf("| Date of Birth    : %-60s |\n", s->dob);
    printf("| Gender           : %-60s |\n", s->gender);
    printf("| Guardian Name    : %-60s |\n", s->guardian_name);
    printf("| Guardian Phone   : %-60s |\n", s->guardian_phone);
    printf("| Address          : %-60s |\n", s->address);
    printf("| Admission Year   : %-60d |\n", s->admission_year);
    printf("| Attendance       : %-58.2f%% |\n", s->attendance);
    printf("| Fee Status       : %-60s |\n", s->fee_status);
    printf("| Total Credits    : %-58.2f |\n", s->total_credits);
    printf("| SGPA             : %-58.2f |\n", s->sgpa);
    printf("+================================================================================+\n");

    printf("| No | Code                 | Course Name          | Cr  | Mark | Grade | GP   |\n");
    printf("+----+----------------------+----------------------+-----+------+-------+------+\n");
    for (i = 0; i < s->course_count; i++)
    {
        printf("| %-2d | %-20s | %-20s | %3.1f | %4.1f | %-5s | %4.2f |\n",
               i + 1,
               s->courses[i].code,
               s->courses[i].name,
               s->courses[i].credit,
               s->courses[i].mark,
               s->courses[i].grade,
               s->courses[i].grade_point);
    }
    printf("+----+----------------------+----------------------+-----+------+-------+------+\n");
}

static void view_single_record_by_id(const char *student_id, int restrict_to_self)
{
    Student s[MAX_STUDENTS];
    int count = load_students(s, MAX_STUDENTS), idx;
    char id[11];

    clear_screen();
    centered_title(restrict_to_self ? "MY ACADEMIC RECORD" : "SEARCH STUDENT RECORD");

    if (count == 0)
    {
        puts("No records found.");
        pause_screen();
        return;
    }

    if (restrict_to_self && student_id)
    {
        strncpy(id, student_id, 10);
        id[10] = '\0';
    }
    else
    {
        get_nonempty_input("Enter student ID: ", id, sizeof(id));
    }

    idx = find_student_index(s, count, id);

    if (idx < 0)
    {
        puts(restrict_to_self
             ? "Your account exists, but your academic record has not been added by admin yet."
             : "Student ID not found.");
        pause_screen();
        return;
    }

    print_student_full_details(&s[idx], idx + 1);
    pause_screen();
}

static void view_user_profile(const char *student_id)
{
    Student s[MAX_STUDENTS];
    int count = load_students(s, MAX_STUDENTS), idx;

    clear_screen();
    centered_title("MY PROFILE");

    idx = find_student_index(s, count, student_id);
    if (idx < 0)
    {
        puts("Profile is not added by admin yet.");
        pause_screen();
        return;
    }

    printf("+================================================================+\n");
    printf("| Student ID       : %-45s |\n", s[idx].student_id);
    printf("| Name             : %-45s |\n", s[idx].name);
    printf("| Department       : %-45s |\n", s[idx].department);
    printf("| Semester         : %-45s |\n", s[idx].semester);
    printf("| Section          : %-45s |\n", s[idx].section);
    printf("| Phone            : %-45s |\n", s[idx].phone);
    printf("| Email            : %-45s |\n", s[idx].email);
    printf("| Date of Birth    : %-45s |\n", s[idx].dob);
    printf("| Gender           : %-45s |\n", s[idx].gender);
    printf("| Guardian Name    : %-45s |\n", s[idx].guardian_name);
    printf("| Guardian Phone   : %-45s |\n", s[idx].guardian_phone);
    printf("| Address          : %-45s |\n", s[idx].address);
    printf("| Admission Year   : %-45d |\n", s[idx].admission_year);
    printf("| Attendance       : %-43.2f%% |\n", s[idx].attendance);
    printf("| Fee Status       : %-45s |\n", s[idx].fee_status);
    printf("+================================================================+\n");

    pause_screen();
}

static void view_all_records(void)
{
    Student s[MAX_STUDENTS];
    int count = load_students(s, MAX_STUDENTS), i;

    clear_screen();
    centered_title("ALL STUDENT RECORDS");

    if (count == 0)
    {
        puts("No records found.");
        pause_screen();
        return;
    }

    print_student_summary_header();
    for (i = 0; i < count; i++) print_student_summary_row(&s[i], i + 1);
    printf("+----+------------+----------------------+-------------+-------------+\n");

    if (get_int_input("\nShow full details? (1=Yes, 2=No): ", 1, 2) == 1)
    {
        int serial = get_int_input("Enter serial number: ", 1, count);
        print_student_full_details(&s[serial - 1], serial);
    }

    pause_screen();
}

static void sort_records(void)
{
    Student s[MAX_STUDENTS], temp;
    int count = load_students(s, MAX_STUDENTS), i, j;

    clear_screen();
    centered_title("SORT RECORDS BY SGPA");

    if (count == 0)
    {
        puts("No records found.");
        pause_screen();
        return;
    }

    for (i = 0; i < count - 1; i++)
    {
        for (j = 0; j < count - i - 1; j++)
        {
            if (s[j].sgpa < s[j + 1].sgpa)
            {
                temp = s[j];
                s[j] = s[j + 1];
                s[j + 1] = temp;
            }
        }
    }

    print_student_summary_header();
    for (i = 0; i < count; i++) print_student_summary_row(&s[i], i + 1);
    printf("+----+------------+----------------------+-------------+-------------+\n");

    pause_screen();
}

static void input_student_profile(Student *n)
{
    get_nonempty_input("Student Name        : ", n->name, sizeof(n->name));
    get_nonempty_input("Department          : ", n->department, sizeof(n->department));
    get_nonempty_input("Semester            : ", n->semester, sizeof(n->semester));
    get_nonempty_input("Section             : ", n->section, sizeof(n->section));
    get_nonempty_input("Phone               : ", n->phone, sizeof(n->phone));
    get_nonempty_input("Email               : ", n->email, sizeof(n->email));
    get_nonempty_input("Date of Birth       : ", n->dob, sizeof(n->dob));
    get_nonempty_input("Gender              : ", n->gender, sizeof(n->gender));
    get_nonempty_input("Guardian Name       : ", n->guardian_name, sizeof(n->guardian_name));
    get_nonempty_input("Guardian Phone      : ", n->guardian_phone, sizeof(n->guardian_phone));
    get_nonempty_input("Address             : ", n->address, sizeof(n->address));
    n->admission_year = get_int_input("Admission Year      : ", 2000, 2100);
    n->attendance = get_float_input("Attendance (0-100)  : ", 0.0f, 100.0f);
    get_nonempty_input("Fee Status (Paid/Due): ", n->fee_status, sizeof(n->fee_status));
}

static void input_course_info(Student *n)
{
    int j;
    float total_credit = 0.0f, total_point = 0.0f;

    n->course_count = get_int_input("Number of courses   : ", 1, MAX_COURSES);

    for (j = 0; j < n->course_count; j++)
    {
        printf("\nCourse %d information\n", j + 1);
        get_nonempty_input("Course Code         : ", n->courses[j].code, sizeof(n->courses[j].code));
        get_nonempty_input("Course Name         : ", n->courses[j].name, sizeof(n->courses[j].name));
        n->courses[j].credit = get_float_input("Credit              : ", 0.5f, 10.0f);
        n->courses[j].mark = get_float_input("Mark (0-100)        : ", 0.0f, 100.0f);
        strcpy(n->courses[j].grade, mark_to_grade(n->courses[j].mark));
        n->courses[j].grade_point = mark_to_grade_point(n->courses[j].mark);

        total_credit += n->courses[j].credit;
        total_point += n->courses[j].grade_point * n->courses[j].credit;
    }

    n->total_credits = total_credit;
    n->sgpa = (total_credit > 0.0f) ? (total_point / total_credit) : 0.0f;
}

static void add_student_record(void)
{
    Student s[MAX_STUDENTS], n;
    int count = load_students(s, MAX_STUDENTS), add_count, i;

    clear_screen();
    centered_title("ADD STUDENT RECORD");

    if (count >= MAX_STUDENTS)
    {
        puts("Student storage is full.");
        pause_screen();
        return;
    }

    add_count = get_int_input("How many students do you want to add? ", 1, MAX_STUDENTS - count);

    for (i = 0; i < add_count; i++)
    {
        memset(&n, 0, sizeof(n));
        clear_screen();
        centered_title("ADD STUDENT RECORD");
        printf("Entering data for student %d\n\n", count + i + 1);

        for (;;)
        {
            get_nonempty_input("Student ID (4-10 digits): ", n.student_id, sizeof(n.student_id));

            if (!is_valid_student_id(n.student_id))
            {
                puts("Invalid student ID format.");
                continue;
            }

            if (find_student_index(s, count + i, n.student_id) >= 0)
            {
                puts("This student ID already exists.");
                continue;
            }

            break;
        }

        input_student_profile(&n);
        input_course_info(&n);

        s[count + i] = n;
        printf("\nCalculated SGPA: %.2f\n", n.sgpa);
        pause_screen();
    }

    count += add_count;

    if (save_students(s, count))
        printf("\nAll records saved successfully in %s\n", STUDENT_FILE);
    else
        puts("\nFailed to save student records.");

    pause_screen();
}

static void update_student_record(void)
{
    Student s[MAX_STUDENTS];
    int count = load_students(s, MAX_STUDENTS), idx;
    char id[11];

    clear_screen();
    centered_title("UPDATE STUDENT RECORD");

    if (count == 0)
    {
        puts("No records found.");
        pause_screen();
        return;
    }

    get_nonempty_input("Enter student ID to update: ", id, sizeof(id));
    idx = find_student_index(s, count, id);

    if (idx < 0)
    {
        puts("Student ID not found.");
        pause_screen();
        return;
    }

    print_student_full_details(&s[idx], idx + 1);
    puts("\nEnter updated information.\n");

    input_student_profile(&s[idx]);
    input_course_info(&s[idx]);

    if (save_students(s, count))
        puts("\nRecord updated successfully.");
    else
        puts("\nFailed to save updated record.");

    pause_screen();
}

static void delete_student_record(void)
{
    Student s[MAX_STUDENTS];
    int count = load_students(s, MAX_STUDENTS), idx, i;
    char id[11];

    clear_screen();
    centered_title("DELETE STUDENT RECORD");

    if (count == 0)
    {
        puts("No records found.");
        pause_screen();
        return;
    }

    get_nonempty_input("Enter student ID to delete: ", id, sizeof(id));
    idx = find_student_index(s, count, id);

    if (idx < 0)
    {
        puts("Student ID not found.");
        pause_screen();
        return;
    }

    for (i = idx; i < count - 1; i++) s[i] = s[i + 1];
    count--;

    if (save_students(s, count))
        puts("Record deleted successfully.");
    else
        puts("Failed to update the file.");

    pause_screen();
}

static void manage_admin_accounts(void)
{
    AdminAccount a[MAX_ADMINS];
    int count = load_admins(a, MAX_ADMINS), i;

    clear_screen();
    centered_title("ADMIN ACCOUNT LIST");

    if (count == 0)
    {
        puts("No admin accounts found.");
        pause_screen();
        return;
    }

    printf("+----+------------------------------+------------------------------+---------------------+\n");
    printf("| SL | Username                     | Created By                   | Created At          |\n");
    printf("+----+------------------------------+------------------------------+---------------------+\n");

    for (i = 0; i < count; i++)
        printf("| %-2d | %-28s | %-28s | %-19s |\n",
               i + 1, a[i].username, a[i].created_by, a[i].created_at);

    printf("+----+------------------------------+------------------------------+---------------------+\n");
    pause_screen();
}

static void admin_register(void)
{
    AdminAccount a[MAX_ADMINS];
    int count, mid;
    char verifier_user[31], verifier_pass[31], new_user[31], new_pass[31];

    clear_screen();
    centered_title("REGISTER NEW ADMIN");
    puts("Verification is required by an existing admin.\n");

    get_nonempty_input("Verifier Admin Username: ", verifier_user, sizeof(verifier_user));
    get_nonempty_input("Verifier Admin Password: ", verifier_pass, sizeof(verifier_pass));

    count = load_admins(a, MAX_ADMINS);
    mid = find_admin_index(a, count, verifier_user);

    if (mid < 0 || strcmp(a[mid].password, verifier_pass) != 0)
    {
        puts("\nVerification failed. New admin cannot be created.");
        pause_screen();
        return;
    }

    get_nonempty_input("New Admin Username: ", new_user, sizeof(new_user));

    if (!is_valid_username(new_user))
    {
        puts("Username must be 3-30 chars and use only letters, digits, '_' or '.'.");
        pause_screen();
        return;
    }

    if (find_admin_index(a, count, new_user) >= 0)
    {
        puts("This admin username already exists.");
        pause_screen();
        return;
    }

    get_nonempty_input("New Admin Password: ", new_pass, sizeof(new_pass));

    if (count >= MAX_ADMINS)
    {
        puts("Admin storage is full.");
        pause_screen();
        return;
    }

    strncpy(a[count].username, new_user, 30);
    a[count].username[30] = '\0';
    strncpy(a[count].password, new_pass, 30);
    a[count].password[30] = '\0';
    strncpy(a[count].created_by, verifier_user, 30);
    a[count].created_by[30] = '\0';
    get_now(a[count].created_at, sizeof(a[count].created_at));
    count++;

    puts(save_admins(a, count) ? "\nNew admin created successfully." : "\nFailed to save admin account.");
    pause_screen();
}

static void admin_change_password(const char *who)
{
    AdminAccount a[MAX_ADMINS];
    int count = load_admins(a, MAX_ADMINS), idx;
    char oldp[31], newp[31];

    clear_screen();
    centered_title("CHANGE ADMIN PASSWORD");

    idx = find_admin_index(a, count, who);
    if (idx < 0)
    {
        puts("Admin account not found.");
        pause_screen();
        return;
    }

    get_nonempty_input("Old Password: ", oldp, sizeof(oldp));
    if (strcmp(a[idx].password, oldp) != 0)
    {
        puts("Old password did not match.");
        pause_screen();
        return;
    }

    get_nonempty_input("New Password: ", newp, sizeof(newp));
    strncpy(a[idx].password, newp, 30);
    a[idx].password[30] = '\0';

    if (save_admins(a, count))
        puts("Admin password changed successfully.");
    else
        puts("Failed to update password.");

    pause_screen();
}

static void user_change_password(const char *student_id)
{
    UserAccount u[MAX_USERS];
    int count = load_users(u, MAX_USERS), idx;
    char oldp[31], newp[31];

    clear_screen();
    centered_title("CHANGE USER PASSWORD");

    idx = find_user_index(u, count, student_id);
    if (idx < 0)
    {
        puts("User account not found.");
        pause_screen();
        return;
    }

    get_nonempty_input("Old Password: ", oldp, sizeof(oldp));
    if (strcmp(u[idx].password, oldp) != 0)
    {
        puts("Old password did not match.");
        pause_screen();
        return;
    }

    get_nonempty_input("New Password: ", newp, sizeof(newp));
    strncpy(u[idx].password, newp, 30);
    u[idx].password[30] = '\0';

    if (save_users(u, count))
        puts("User password changed successfully.");
    else
        puts("Failed to update password.");

    pause_screen();
}

static void view_login_history(const char *filename, const char *title_text)
{
    FILE *fp = fopen(filename, "r");
    char line[256];

    clear_screen();
    centered_title(title_text);

    if (!fp)
    {
        puts("No history file found yet.");
        pause_screen();
        return;
    }

    printf("+---------------------+----------+------------------------------+------------------+\n");
    printf("| Time                | Role     | ID/Username                  | Status           |\n");
    printf("+---------------------+----------+------------------------------+------------------+\n");

    while (fgets(line, sizeof(line), fp))
        if (line[0] == '|') printf("%s", line);

    printf("+---------------------+----------+------------------------------+------------------+\n");
    fclose(fp);

    pause_screen();
}

static void admin_menu(const char *who)
{
    char info[100];

    for (;;)
    {
        int ch;

        clear_screen();
        centered_title("ADMIN MENU");
        printf("\n");

        snprintf(info, sizeof(info), "Logged in as: %s", who);
        print_menu_line(info, 40);
        printf("\n");

        print_menu_line("1. Add Student Record", 40);
        print_menu_line("2. Update Student Record", 40);
        print_menu_line("3. View All Records", 40);
        print_menu_line("4. Sort Records by SGPA", 40);
        print_menu_line("5. Search Student Record", 40);
        print_menu_line("6. Delete Student Record", 40);
        print_menu_line("7. View User Login History", 40);
        print_menu_line("8. View Admin Login History", 40);
        print_menu_line("9. View Admin Accounts", 40);
        print_menu_line("10. Register New Admin", 40);
        print_menu_line("11. Change Admin Password", 40);
        print_menu_line("12. Logout", 40);
        printf("\n");

        ch = get_int_input("Enter your choice (1-12): ", 1, 12);

        switch (ch)
        {
        case 1: add_student_record(); break;
        case 2: update_student_record(); break;
        case 3: view_all_records(); break;
        case 4: sort_records(); break;
        case 5: view_single_record_by_id(NULL, 0); break;
        case 6: delete_student_record(); break;
        case 7: view_login_history(USER_LOGIN_FILE, "USER LOGIN HISTORY"); break;
        case 8: view_login_history(ADMIN_LOGIN_FILE, "ADMIN LOGIN HISTORY"); break;
        case 9: manage_admin_accounts(); break;
        case 10: admin_register(); break;
        case 11: admin_change_password(who); break;
        case 12: return;
        }
    }
}

static void admin_login(void)
{
    AdminAccount a[MAX_ADMINS];
    char username[31], password[31];
    int count, idx;

    clear_screen();
    centered_title("ADMIN LOGIN");

    get_nonempty_input("Username: ", username, sizeof(username));
    get_nonempty_input("Password: ", password, sizeof(password));

    count = load_admins(a, MAX_ADMINS);
    idx = find_admin_index(a, count, username);

    if (idx >= 0 && strcmp(a[idx].password, password) == 0)
    {
        append_login_history(ADMIN_LOGIN_FILE, "ADMIN", username, "SUCCESS");
        printf("\nLogin successful. Welcome, %s.\n", username);
        pause_screen();
        admin_menu(username);
    }
    else
    {
        append_login_history(ADMIN_LOGIN_FILE, "ADMIN", username, "FAILED");
        puts("\nInvalid admin username or password.");
        pause_screen();
    }
}

static void user_register(void)
{
    UserAccount u[MAX_USERS];
    int count;
    char id[11], password[31];

    clear_screen();
    centered_title("NEW USER REGISTRATION");

    get_nonempty_input("Enter Student ID (4-10 digits): ", id, sizeof(id));

    if (!is_valid_student_id(id))
    {
        puts("Invalid student ID format.");
        pause_screen();
        return;
    }

    count = load_users(u, MAX_USERS);

    if (find_user_index(u, count, id) >= 0)
    {
        puts("This user is already registered. Please login.");
        pause_screen();
        return;
    }

    get_nonempty_input("Create Password: ", password, sizeof(password));

    if (count >= MAX_USERS)
    {
        puts("User storage is full.");
        pause_screen();
        return;
    }

    strncpy(u[count].student_id, id, 10);
    u[count].student_id[10] = '\0';
    strncpy(u[count].password, password, 30);
    u[count].password[30] = '\0';
    get_now(u[count].created_at, sizeof(u[count].created_at));
    count++;

    if (save_users(u, count))
    {
        puts("\nRegistration successful. You can now login.");
        append_login_history(USER_LOGIN_FILE, "USER", id, "REGISTERED");
    }
    else
    {
        puts("\nFailed to save user account.");
    }

    pause_screen();
}

static void user_menu(const char *id)
{
    char info[100];

    for (;;)
    {
        int ch;

        clear_screen();
        centered_title("USER MENU");
        printf("\n");

        snprintf(info, sizeof(info), "Logged in as Student ID: %s", id);
        print_menu_line(info, 42);
        printf("\n");

        print_menu_line("1. View My Profile", 42);
        print_menu_line("2. View My Academic Record", 42);
        print_menu_line("3. Search My Academic Record", 42);
        print_menu_line("4. Change Password", 42);
        print_menu_line("5. Logout", 42);
        printf("\n");

        ch = get_int_input("Enter your choice (1-5): ", 1, 5);

        switch (ch)
        {
        case 1: view_user_profile(id); break;
        case 2: view_single_record_by_id(id, 1); break;
        case 3: view_single_record_by_id(id, 1); break;
        case 4: user_change_password(id); break;
        case 5: return;
        }
    }
}

static void user_login(void)
{
    UserAccount u[MAX_USERS];
    int count, idx;
    char id[11], password[31];

    clear_screen();
    centered_title("USER LOGIN");

    get_nonempty_input("Enter Student ID: ", id, sizeof(id));
    get_nonempty_input("Enter Password : ", password, sizeof(password));

    count = load_users(u, MAX_USERS);
    idx = find_user_index(u, count, id);

    if (idx < 0)
    {
        append_login_history(USER_LOGIN_FILE, "USER", id, "FAILED_NOT_REGISTERED");
        puts("\nThis user is not registered yet. Please register first.");
        pause_screen();
        return;
    }

    if (strcmp(u[idx].password, password) != 0)
    {
        append_login_history(USER_LOGIN_FILE, "USER", id, "FAILED_WRONG_PASSWORD");
        puts("\nWrong password.");
        pause_screen();
        return;
    }

    append_login_history(USER_LOGIN_FILE, "USER", id, "SUCCESS");
    puts("\nLogin successful.");
    pause_screen();
    user_menu(id);
}

static void admin_entry(void)
{
    for (;;)
    {
        int ch;

        clear_screen();
        centered_title("ADMIN PANEL");
        printf("\n");

        print_menu_line("1. Admin Login", 34);
        print_menu_line("2. Register New Admin", 34);
        print_menu_line("3. Main Menu", 34);
        printf("\n");

        ch = get_int_input("Enter your choice (1-3): ", 1, 3);

        if (ch == 1) admin_login();
        else if (ch == 2) admin_register();
        else return;
    }
}

static void user_entry(void)
{
    for (;;)
    {
        int ch;

        clear_screen();
        centered_title("USER PANEL");
        printf("\n");

        print_menu_line("1. User Login", 34);
        print_menu_line("2. New User Registration", 34);
        print_menu_line("3. Main Menu", 34);
        printf("\n");

        ch = get_int_input("Enter your choice (1-3): ", 1, 3);

        if (ch == 1) user_login();
        else if (ch == 2) user_register();
        else return;
    }
}

static void developer_info(void)
{
    clear_screen();
    centered_title("ABOUT PROJECT / DEVELOPERS");
    printf("\n");

    print_menu_line("Project: Student Management System for University", 68);
    print_menu_line("Language: C Programming", 68);
    print_menu_line("Storage: Structured Text Files", 68);
    printf("\n");
    print_menu_line("Developer Name : Md. Alif Rabbi", 68);
    print_menu_line("Student ID     : 232-15-601", 68);
    print_menu_line("Department     : CSE, DIU", 68);
    printf("\n");
    print_menu_line("Features: Admin, User, Student Profile, Result, SGPA,", 68);
    print_menu_line("Attendance, Fee Status, Login History, Password Change", 68);

    pause_screen();
}

static void main_menu(void)
{
    for (;;)
    {
        int ch;

        clear_screen();
        centered_title("MAIN MENU");
        printf("\n");

        center_line("+--------------------------------------------------------------+");
        center_line("|                     |                    |                   |");
        center_line("|    1:  ADMIN        |    2:  USER        |   3:  DEVELOPER   |");
        center_line("|                     |                    |                   |");
        center_line("+--------------------------------------------------------------+");
        center_line("|                           4:  EXIT                           |");
        center_line("+--------------------------------------------------------------+");
        printf("\n");

        ch = get_int_input("Enter your choice (1-4): ", 1, 4);

        if (ch == 1) admin_entry();
        else if (ch == 2) user_entry();
        else if (ch == 3) developer_info();
        else
        {
            clear_screen();
            center_line("Thank you for using the Student Management System.");
            return;
        }
    }
}

int main(void)
{
    ensure_default_admin();

    clear_screen();
    printf("\n\n\n");
    center_line("+--------------------------------------------------------+");
    center_line("|                                                        |");
    center_line("|                      WELCOME TO                        |");
    center_line("|               Student Management System                |");
    center_line("|                                                        |");
    center_line("+--------------------------------------------------------+");
    printf("\n");
    center_line("Press Enter...");
    fflush(stdout);

    wait_for_enter();
    main_menu();

    return 0;
}
