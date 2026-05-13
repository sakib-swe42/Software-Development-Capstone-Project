#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SHIFT_LIMIT 20
#define MAX_LINE    512

void thankYouPopup(void)
{
    printf("\n  ------------------------------\n");
    printf(" |       Thanks For Using       |\n");
    printf(" | Healthcare Management System |\n");
    printf(" |   Stay healthy, Stay happy!  |\n");
    printf("  ------------------------------\n\n");
}


typedef struct
{
    char username[50];
    char mobile[20];
    char password[50];
}
Patient;

typedef struct
{
    char username[50];
    char password[50];
    char mobile[20];
    char nid[20];
    char education[200];
    char name[50];
    char specialty[80];
    char shift1[20];
    char shift2[20];
    char location[100];
}
Doctor;


typedef struct
{
    char username[50];
    char mobile[20];
    char password[50];
    char fullName[50];
    char nid[20];
    char hospitalName[80];
    char hospitalLocation[80];
}
Admin;


typedef struct
{
    char doctorUsername[50];
    char patientUsername[50];
    char date[20];
    int  shift;
} Appointment;

void initDefaults(void);
void ensureFile(const char *filename);
int  fileExists(const char *filename);
int  fileIsEmpty(const char *filename);
int  count_commas(const char *s);
void migrateDoctorsFileTo10FieldsIfNeeded(void);
void migrateAdminsFileTo7FieldsIfNeeded(void);
void seedDoctorsIfEmpty(void);
void seedDefaultAccounts(void);

int  nextDoctor(FILE *f, Doctor *d);
int  nextAdmin(FILE *f, Admin *a);

void mainMenu(void);
void flush_input(void);

void patientPortal(void);
void patientRegister(void);
void patientLogin(void);
void patientForgetPassword(void);
void patientMenu(Patient *logged);
void searchDoctor_forPatient(void);
int  doctorExists(const char *doctorUsername);
int  countPatientsInShiftOnDate(const char *doctorUsername, int shift, const char *date);
void bookAppointment(Patient *logged);
void viewReminders(Patient *logged);


void doctorPortal(void);
void doctorRegister(void);
void doctorLogin(void);
void doctorForgetPassword(void);
void doctorMenu(Doctor *logged);
void doctorViewMyAppointments(Doctor *logged);
void doctorReschedule(Doctor *logged);
void doctorAddReminder(Doctor *logged);


void adminPortal(void);
void adminRegister(void);
void adminForgetPassword(void);
void adminLogin(void);
void adminMenu(char *adminUser);
void adminViewAppointments(void);
void adminEditAppointment(void);
void adminManageDoctors(void);
void adminAddDoctor(void);
void adminDeleteDoctor(void);

int main(void)
{
    initDefaults();
    mainMenu();
    return 0;
}

void ensureFile(const char *filename)
{
    if (!fileExists(filename))
    {
        FILE *f = fopen(filename, "w");
        if (f) fclose(f);
    }
}

int fileExists(const char *filename)
{
    FILE *f = fopen(filename, "r");
    if (f)
        {
             fclose(f);
             return 1;
        }
    return 0;
}

int fileIsEmpty(const char *filename)
{
    FILE *f = fopen(filename, "r");
    if (!f)
    return 1;
    int c = fgetc(f);
    fclose(f);
    return (c == EOF);
}

int count_commas(const char *s) {
    int c = 0;
    while (*s)
    {
        if (*s == ',') c++; s++;
    }
    return c;
}

void migrateDoctorsFileTo10FieldsIfNeeded(void)
{
    FILE *in = fopen("doctors.txt", "r");
    if (!in)
    return;
    char line[MAX_LINE];
    int needs = 0;
    while (fgets(line, sizeof(line), in))
    {
        int commas = count_commas(line);
        if (commas == 5 || commas == 7)
        {
            needs = 1;
            break;
        }
    }
    fclose(in);
    if (!needs)
    return;

    in = fopen("doctors.txt", "r");
    FILE *out = fopen("temp_doctors_migrate.txt", "w");
    if (!in || !out)
    {
        if(in) fclose(in); if(out) fclose(out);
        return;
    }

    while (fgets(line, sizeof(line), in))
    {
        Doctor d;
        if (sscanf(line, "%49[^,],%49[^,],%19[^,],%19[^,],%79[^,],%49[^,],%49[^,],%19[^,],%19[^,],%49[^\n]",
                   d.username, d.password, d.mobile, d.nid, d.education,
                   d.name, d.specialty, d.shift1, d.shift2, d.location) == 10)
                  {
                    fprintf(out, "%s,%s,%s,%s,%s,%s,%s,%s,%s,%s\n",
                    d.username, d.password, d.mobile, d.nid, d.education,
                    d.name, d.specialty, d.shift1, d.shift2, d.location);
                    continue;
                  }
        {
            char u[50], pw[50], m[20], name[50], spec[50], sh1[20], sh2[20], loc[50];
            if (sscanf(line, "%49[^,],%49[^,],%19[^,],%49[^,],%49[^,],%19[^,],%19[^,],%49[^\n]",
                       u, pw, m, name, spec, sh1, sh2, loc) == 8)
                  {
                    fprintf(out, "%s,%s,%s,%s,%s,%s,%s,%s,%s,%s\n",
                        u, pw, m, "NID0000000000", "Unknown",
                        name, spec, sh1, sh2, loc);
                        continue;
                  }
        }

        {
            char u[50], name[50], spec[50], sh1[20], sh2[20], loc[50];
            if (sscanf(line, "%49[^,],%49[^,],%49[^,],%19[^,],%19[^,],%49[^\n]",
                       u, name, spec, sh1, sh2, loc) == 6)
                      {
                        fprintf(out, "%s,%s,%s,%s,%s,%s,%s,%s,%s,%s\n",
                        u, "doc123", "01700000000", "NID0000000000", "Unknown",
                        name, spec, sh1, sh2, loc);
                        continue;
                      }
        }

    }
    fclose(in);
    fclose(out);
    remove("doctors.txt");
    rename("temp_doctors_migrate.txt", "doctors.txt");
}


void migrateAdminsFileTo7FieldsIfNeeded(void)
{
    FILE *in = fopen("admins.txt", "r");
    if (!in)
    return;
    char line[MAX_LINE];
    int needs = 0;
    while (fgets(line, sizeof(line), in))
    {
        int commas = count_commas(line);
        if (commas == 2)
        {
            needs = 1;
            break;
        }
    }
    fclose(in);
    if (!needs)
    return;

    in = fopen("admins.txt", "r");
    FILE *out = fopen("temp_admins_migrate.txt", "w");
    if (!in || !out)
    {
        if(in) fclose(in); if(out) fclose(out);
        return;
    }

    while (fgets(line, sizeof(line), in))
    {
        Admin a;
        if (sscanf(line, "%49[^,],%19[^,],%49[^,],%49[^,],%19[^,],%79[^,],%79[^\n]",
                   a.username, a.mobile, a.password,
                   a.fullName, a.nid, a.hospitalName, a.hospitalLocation) == 7)
                   {
                    fprintf(out, "%s,%s,%s,%s,%s,%s,%s\n",
                    a.username, a.mobile, a.password,
                    a.fullName, a.nid, a.hospitalName, a.hospitalLocation);
                    continue;
                   }

        {
            char u[50], m[20], p[50];
            if (sscanf(line, "%49[^,],%19[^,],%49[^\n]", u, m, p) == 3)
            {
                fprintf(out, "%s,%s,%s,%s,%s,%s,%s\n",
                        u, m, p, "Admin User", "NID0000000000", "Default Hospital", "Default Location");
                continue;
            }
        }
    }
    fclose(in);
    fclose(out);
    remove("admins.txt");
    rename("temp_admins_migrate.txt", "admins.txt");
}

void seedDoctorsIfEmpty(void)
{
    if (!fileIsEmpty("doctors.txt"))
        return;

    FILE *f = fopen("doctors.txt", "w");
    if (!f)
        return;


    fprintf(f, "dr_hridoy,hridoy123,01721567261,NID16458945651,MBBS from DU | FCPS from BSMMU,Dr. Rashedul Hasan,Cardiology,9AM-12PM,2PM-5PM,ALOK Medical - Mirpur-10 - Dhaka\n");
    fprintf(f, "dr_sakib,doc123,01720000001,NID1451315411,MBBS from Chittagong Medical College | MD from BSMMU,Dr. Sakib,Cardiology,9AM-12PM,2PM-5PM,Labaid Hospital - Mirpur-1 - Dhaka-1216\n");
    fprintf(f, "dr_rahman,doc123,01720000002,NID115151512152,BAMS from Noakhali Medical | MPH from BRAC University,Dr. Rahman,Dermatology,10AM-1PM,3PM-6PM,IBN Sina Medical - Kallanpur - Dhaka\n");
    fprintf(f, "dr_alam,doc123,01720000003,NID545151234103,BHMS from Dhaka Medical | MD from BSMMU,Dr. Alam,Neurology,8AM-11AM,1PM-4PM,United Hospital - Gulshan-2 - Dhaka-1212\n");
    fprintf(f, "dr_sumi,doc123,01720000004,NID14241414104,BDS from Rajshahi Medical College | MDS from Dhaka Dental College,Dr. Sumi,Medicine,9AM-12PM,2PM-5PM,Square Hospitals Ltd - Panthapath - Dhaka-1205\n");
    fprintf(f, "dr_raju,doc123,01720000005,NID3764958246405,MBBS from Sylhet MAG Osmani Medical College | FCPS from BCPS,Dr. Raju,Orthopedics,10AM-1PM,3PM-6PM,Evercare Hospital - Bashundhara R/A - Dhaka-1229\n");
    fprintf(f, "dr_nadia,doc123,01720000006,NID55454145416,MBBS from Rangpur Medical College | Diploma in Cardiology from BSMMU,Dr. Nadia,Gynecology,9AM-12PM,2PM-5PM,Chittagong Medical Center - Agrabad - Chattogram\n");
    fprintf(f, "dr_tanvir,doc123,01720000007,NID75414575457,BDS from Chittagong Medical College | MDS from BSMMU,Dr. Tanvir,Pediatrics,9AM-12PM,2PM-5PM,Chittagong Medical Center - Agrabad - Chattogram\n");
    fprintf(f, "dr_sajib,doc123,01720000008,NID175754541458,BDS from Chittagong Medical College | MDS from BSMMU,Dr. Sajib,ENT,10AM-1PM,3PM-6PM,Evercare Hospital - Bashundhara R/A - Dhaka-1229\n");
    fprintf(f, "dr_mitu,doc123,01720000009,NID154515181219,BAMS from Noakhali Medical | MPH from BRAC University,Dr. Mitu,Ophthalmology,8AM-11AM,1PM-4PM,ALOK Medical - Mirpur-10 - Dhaka\n");
    fprintf(f, "dr_bijoy,doc123,01720000010,NID14154161215,MBBS from Sylhet MAG Osmani Medical College | FCPS from BCPS,Dr. Bijoy,Nephrology,9AM-12PM,2PM-5PM,Gazipur\n");
    fprintf(f, "dr_farhan,doc123,01720000011,NID18794561311,MBBS from Rangpur Medical College | Diploma in Cardiology from BSMMU,Dr. Farhan,Endocrinology,10AM-1PM,3PM-6PM,Square Hospitals Ltd - Panthapath - Dhaka-1205\n");
    fprintf(f, "dr_aysha,doc123,01720000012,NID5678942312,BDS from Rajshahi Medical College | MDS from Dhaka Dental College,Dr. Aysha,Oncology,9AM-12PM,2PM-5PM,United Hospital - Gulshan-2 - Dhaka-1212\n");
    fprintf(f, "dr_rifat,doc123,01720000013,NID8795631413,BHMS from Dhaka Medical | MD from BSMMU,Dr. Rifat,Gastroenterology,9AM-12PM,2PM-5PM,IBN Sina Medical - Kallanpur - Dhaka\n");
    fprintf(f, "dr_shila,doc123,01720000014,NID15649782314,MBBS from Chittagong Medical College | MD from BSMMU,Dr. Shila,Psychiatry,10AM-1PM,3PM-6PM,Labaid Hospital - Mirpur-1 - Dhaka-1216\n");
    fprintf(f, "dr_sabbir,doc123,01720000015,NID184548421512115,MBBS from DU | FCPS from BSMMU,Dr. Sabbir,Cardiology,9AM-12PM,2PM-5PM,ALOK Medical - Mirpur-10 - Dhaka\n");

    fclose(f);
}



void seedDefaultAccounts(void)
{
    int hasPatient = 0;
    FILE *pf = fopen("patients.txt", "r");
    if (pf)
    {
        char u[50], m[20], pw[50];
        while (fscanf(pf, "%49[^,],%19[^,],%49[^\n]\n", u, m, pw) == 3)
        {
            if (strcmp(u, "patient1") == 0)
            {
                hasPatient = 1;
                break;
            }
        }
        fclose(pf);
    }
    if (!hasPatient)
    {
        pf = fopen("patients.txt", "a");
        if (pf)
        {
            fprintf(pf, "patient1,01733330000,pat123\n"); fclose(pf);
        }
    }

    int hasAdmin = 0;
    FILE *af = fopen("admins.txt", "r");
    if (af)
    {
        Admin a;
        while (nextAdmin(af, &a))
        {
            if (strcmp(a.username, "admin") == 0)
            {
                hasAdmin = 1;
                break;
            }
        }
        fclose(af);
    }
    if (!hasAdmin)
    {
        af = fopen("admins.txt", "a");
        if (af)
        {
            fprintf(af, "admin,01711110000,admin123,Default Admin,NID999999999999,City Hospital,Dhaka\n");
            fclose(af);
        }
    }
}

void initDefaults(void)
{
    ensureFile("patients.txt");
    ensureFile("doctors.txt");
    ensureFile("admins.txt");
    ensureFile("appointments.txt");
    ensureFile("reminders.txt");

    migrateDoctorsFileTo10FieldsIfNeeded();
    migrateAdminsFileTo7FieldsIfNeeded();
    seedDoctorsIfEmpty();
    seedDefaultAccounts();
}

int nextDoctor(FILE *f, Doctor *d)
{
    char line[MAX_LINE];
    while (fgets(line, sizeof(line), f))
    {
    
        if (sscanf(line, "%49[^,],%49[^,],%19[^,],%19[^,],%79[^,],%49[^,],%49[^,],%19[^,],%19[^,],%49[^\n]",
                   d->username, d->password, d->mobile, d->nid, d->education,
                   d->name, d->specialty, d->shift1, d->shift2, d->location) == 10)
            {
                return 1;
            }

        {
            char u[50], pw[50], m[20], name[50], spec[50], sh1[20], sh2[20], loc[50];
            if (sscanf(line, "%49[^,],%49[^,],%19[^,],%49[^,],%49[^,],%19[^,],%19[^,],%49[^\n]",
                       u, pw, m, name, spec, sh1, sh2, loc) == 8)
                {
                    strcpy(d->username, u);
                strcpy(d->password, pw);
                strcpy(d->mobile,   m);
                strcpy(d->nid, "NID0000000000");
                strcpy(d->education, "Unknown");
                strcpy(d->name, name);
                strcpy(d->specialty, spec);
                strcpy(d->shift1, sh1);
                strcpy(d->shift2, sh2);
                strcpy(d->location, loc);
                return 1;
                }
        }

        {
            char u[50], name[50], spec[50], sh1[20], sh2[20], loc[50];
            if (sscanf(line, "%49[^,],%49[^,],%49[^,],%19[^,],%19[^,],%49[^\n]",
                       u, name, spec, sh1, sh2, loc) == 6)
                       {
                        strcpy(d->username, u);
                        strcpy(d->password, "doc123");
                        strcpy(d->mobile,   "01700000000");
                        strcpy(d->nid, "NID0000000000");
                        strcpy(d->education, "Unknown");
                        strcpy(d->name, name);
                        strcpy(d->specialty, spec);
                        strcpy(d->shift1, sh1);
                        strcpy(d->shift2, sh2);
                        strcpy(d->location, loc);
                        return 1;
                       }

        }
    }
    return 0;
}

int nextAdmin(FILE *f, Admin *a)
{
    char line[MAX_LINE];
    while (fgets(line, sizeof(line), f))
    {
        if (sscanf(line, "%49[^,],%19[^,],%49[^,],%49[^,],%19[^,],%79[^,],%79[^\n]",
                   a->username, a->mobile, a->password,
                   a->fullName, a->nid, a->hospitalName, a->hospitalLocation) == 7)
                   {
                     return 1;
                   }

        {
            char u[50], m[20], p[50];
            if (sscanf(line, "%49[^,],%19[^,],%49[^\n]", u, m, p) == 3)
            {
                strcpy(a->username, u);
                strcpy(a->mobile, m);
                strcpy(a->password, p);
                strcpy(a->fullName, "Admin User");
                strcpy(a->nid, "NID0000000000");
                strcpy(a->hospitalName, "Default Hospital");
                strcpy(a->hospitalLocation, "Default Location");
                return 1;
            }
        }

    }
    return 0;
}

void mainMenu(void)
{
    int choice;
    while (1)
    {
        printf("\n||======= WELCOME TO =======||\n\n");
        printf(" HEALTHCARE MANAGEMENT SYSTEM\n------------------------------\n\n");
        printf("Tell me are you a_\n");
        printf("1) Patient\n");
        printf("2) Doctor\n");
        printf("3) Hospital Admin\n");
        printf("0) Exit\n");
        printf("Enter choice: ");
        if (scanf("%d", &choice) != 1)
        {
            flush_input();
            continue;
        }
        getchar();
        if      (choice == 1) patientPortal();
        else if (choice == 2) doctorPortal();
        else if (choice == 3) adminPortal();
        else if (choice == 0)
        {
            printf("Goodbye!\n");
            break;
        }
        else printf("Invalid choice!\n");
    }
}

void patientPortal(void)
{
    int choice;
    while (1)
    {
        printf("\n--- Patient Portal ---\n");
        printf("1) Login\n");
        printf("2) New User? Register Now!\n");
        printf("3) Forget Password\n");
        printf("0) Back\n");
        printf("Enter choice: ");
        if (scanf("%d", &choice) != 1)
        {
            flush_input();
            continue;
        }
        getchar();
        if      (choice == 1) patientLogin();
        else if (choice == 2) patientRegister();
        else if (choice == 3) patientForgetPassword();
        else if (choice == 0)
        return;
        else printf("Invalid choice!\n");
    }
}

void patientRegister(void)
{
    Patient p;
    printf("Enter username: ");  scanf("%49s", p.username);
    printf("Enter mobile: ");    scanf("%19s", p.mobile);
    printf("Enter password: ");  scanf("%49s", p.password);

    FILE *f = fopen("patients.txt", "r");
    if (!f)
    {
        printf("File error!\n");
        return;
    }
    char u[50], m[20], pw[50];
    while (fscanf(f, "%49[^,],%19[^,],%49[^\n]\n", u, m, pw) == 3)
    {
        if (strcmp(u, p.username) == 0)
        {
            printf("Username already exists!\n"); fclose(f);
            return;
        }
    }
    fclose(f);

    f = fopen("patients.txt", "a");
    if (!f)
    {
        printf("File error!\n");
        return;
    }
    fprintf(f, "%s,%s,%s\n", p.username, p.mobile, p.password);
    fclose(f);
    printf("\n****************************\n");
    printf("* Registration successful! *\n");
    printf("****************************\n");
}

void patientLogin(void)
{
    Patient input, temp;
    printf("Enter username: ");  scanf("%49s", input.username);
    printf("Enter password: ");  scanf("%49s", input.password);

    FILE *f = fopen("patients.txt", "r");
    if (!f)
    {
        printf("File error!\n");
        return;
    }
    int ok = 0;
    while (fscanf(f, "%49[^,],%19[^,],%49[^\n]\n", temp.username, temp.mobile, temp.password) == 3)
    {
        if (strcmp(temp.username, input.username) == 0 &&
            strcmp(temp.password, input.password) == 0)
            {
                ok = 1;
                break;
            }
    }
    fclose(f);
    if (!ok)
    {
        printf("Invalid username or password!\n");
        return;
    }
    printf("\nLogin successful!\n");
    patientMenu(&temp);
}

void patientForgetPassword(void)
{
    char uname[50], mobile[20], newpass[50];
    printf("Enter username: "); scanf("%49s", uname);
    printf("Enter mobile: ");   scanf("%19s", mobile);

    FILE *f = fopen("patients.txt", "r");
    FILE *t = fopen("temp_pat.txt", "w");
    if (!f || !t)
    {
        printf("File error!\n"); if(f)fclose(f); if(t)fclose(t);
        return;
    }

    Patient p; int found = 0;
    while (fscanf(f, "%49[^,],%19[^,],%49[^\n]\n", p.username, p.mobile, p.password) == 3)
    {
        if (strcmp(p.username, uname) == 0 && strcmp(p.mobile, mobile) == 0)
        {
            printf("Enter new password: "); scanf("%49s", newpass);
            fprintf(t, "%s,%s,%s\n", p.username, p.mobile, newpass);
            found = 1;
        }
        else
        {
            fprintf(t, "%s,%s,%s\n", p.username, p.mobile, p.password);
        }
    }
    fclose(f); fclose(t);
    remove("patients.txt");
    rename("temp_pat.txt", "patients.txt");
    if (found)
        {
            printf("\n**********************************\n");
        printf("* Password changed successfully! *\n");
        printf("**********************************\n");
        }
    else
    {
        printf("Username or mobile not found!\n");
    }
}

void patientMenu(Patient *logged)
{
    int choice;
    while (1)
    {
        printf("\n--- Patient Menu (%s) ---\n", logged->username);
        printf("1) Search Doctor\n");
        printf("2) Book Appointment\n");
        printf("3) View Reminders\n");
        printf("0) Logout\n");
        printf("Enter choice: ");
        if (scanf("%d", &choice) != 1)
        {
            flush_input();
            continue;
        }
        getchar();

        if      (choice == 1) searchDoctor_forPatient();
        else if (choice == 2) bookAppointment(logged);
        else if (choice == 3) viewReminders(logged);
        else if (choice == 0)
        return;
        else printf("Invalid choice!\n");
    }
}

void searchDoctor_forPatient(void)
{
    FILE *f = fopen("doctors.txt", "r");
    if (!f)
    {
        printf("File error!\n");
        return;
    }
    Doctor d;
    printf("\n----- Doctor List -----\n");
    while (nextDoctor(f, &d))
    {
        printf("Username  : %s\n", d.username);
        printf("Name      : %s\n", d.name);
        printf("NID       : %s\n", d.nid);
        printf("Education : %s\n", d.education);
        printf("Specialty : %s\n", d.specialty);
        printf("Shift 1   : %s\n", d.shift1);
        printf("Shift 2   : %s\n", d.shift2);
        printf("Location  : %s\n\n", d.location);
    }
    fclose(f);
}

int doctorExists(const char *doctorUsername)
{
    FILE *f = fopen("doctors.txt", "r");
    if (!f) return 0;
    Doctor d; int ok = 0;
    while (nextDoctor(f, &d))
    {
        if (strcmp(d.username, doctorUsername) == 0)
        {
            ok = 1;
            break;
        }
    }
    fclose(f);
    return ok;
}

int countPatientsInShiftOnDate(const char *doctorUsername, int shift, const char *date)
{
    FILE *f = fopen("appointments.txt", "r");
    if (!f)
    return 0;
    Appointment a; int count = 0;
    while (fscanf(f, "%49[^,],%49[^,],%19[^,],%d\n",
                  a.doctorUsername, a.patientUsername, a.date, &a.shift) == 4)
                 {
                    if (strcmp(a.doctorUsername, doctorUsername) == 0 &&
            a.shift == shift &&
            strcmp(a.date, date) == 0)
          {
            count++;
          }
                 }
    fclose(f);
    return count;
}

void bookAppointment(Patient *logged)
{
    Appointment a;
    printf("Enter doctor username: "); scanf("%49s", a.doctorUsername);
    if (!doctorExists(a.doctorUsername))
    {
        printf("Doctor not found!\n");
        return;
    }
    printf("Enter date (DD-MM-YYYY): ");
    scanf("%19s", a.date);
    printf("Enter shift (1 or 2): ");
    scanf("%d", &a.shift);
    getchar();

    if (a.shift != 1 && a.shift != 2)
    {
        printf("Invalid shift!\n");
        return;
    }

    int curr = countPatientsInShiftOnDate(a.doctorUsername, a.shift, a.date);
    if (curr >= SHIFT_LIMIT)
    {
        printf("This shift is full (20 patients). Choose another shift/date.\n");
        return;
    }

    strcpy(a.patientUsername, logged->username);
    FILE *f = fopen("appointments.txt", "a");
    if (!f)
    {
        printf("File error!\n");
        return;
    }
    fprintf(f, "%s,%s,%s,%d\n", a.doctorUsername, a.patientUsername, a.date, a.shift);
    fclose(f);

    FILE *r = fopen("reminders.txt", "a");
    if (r)
    {
        fprintf(r, "Reminder for %s: Appointment with %s on %s (Shift %d)\n",
                a.patientUsername, a.doctorUsername, a.date, a.shift);
        fclose(r);
    }

    printf("\n||  Appointment booked! Soon you will get a confirmation message. ||\n");
    thankYouPopup();
}

void viewReminders(Patient *logged)
{
    FILE *f = fopen("reminders.txt", "r");
    if (!f)
    {
        printf("No reminders yet.\n");
        return;
    }
    char line[256];
    int shown = 0;
    printf("\n----- Reminders for %s -----\n", logged->username);
    while (fgets(line, sizeof(line), f))
    {
        char tag[80]; snprintf(tag, sizeof(tag), "Reminder for %s:", logged->username);
        if (strstr(line, tag))
        {
             printf("%s", line);
             shown = 1;
        }
    }
    fclose(f);
    if (!shown)
    printf("No reminders found.\n");
}


void doctorPortal(void)
{
    int choice;
    while (1)
    {
        printf("\n--- Doctor Portal ---\n");
        printf("1) Login\n");
        printf("2) New User? Register Now!\n");
        printf("3) Forget Password\n");
        printf("0) Back\n");
        printf("Enter choice: ");
        if (scanf("%d", &choice) != 1)
        {
            flush_input();
            continue;
        }
        getchar();

        if      (choice == 1) doctorLogin();
        else if (choice == 2) doctorRegister();
        else if (choice == 3) doctorForgetPassword();
        else if (choice == 0)
        return;
        else
        printf("Invalid choice!\n");
    }
}

void doctorRegister(void)
{
    Doctor d;
    printf("Choose username: ");   scanf("%49s", d.username);
    printf("Enter password: ");    scanf("%49s", d.password);
    printf("Enter mobile: ");      scanf("%19s", d.mobile);
    printf("Enter NID: ");         scanf("%19s", d.nid);
    printf("Enter education: ");   scanf(" %79[^\n]", d.education);
    printf("Enter full name: ");   scanf(" %49[^\n]", d.name);
    printf("Enter specialty: ");   scanf(" %49[^\n]", d.specialty);
    printf("Enter shift1: ");      scanf(" %19[^\n]", d.shift1);
    printf("Enter shift2: ");      scanf(" %19[^\n]", d.shift2);
    printf("Enter location: ");    scanf(" %49[^\n]", d.location);

    FILE *f = fopen("doctors.txt", "r");
    if (!f)
    {
        printf("File error!\n");
        return;
    }
    Doctor tmp;
    while (nextDoctor(f, &tmp))
    {
        if (strcmp(tmp.username, d.username) == 0)
        {
             printf("Username already exists!\n"); fclose(f);
             return;
        }
    }
    fclose(f);

    f = fopen("doctors.txt", "a");
    if (!f)
    {
         printf("File error!\n");
         return;
    }
    fprintf(f, "%s,%s,%s,%s,%s,%s,%s,%s,%s,%s\n",
            d.username, d.password, d.mobile, d.nid, d.education,
            d.name, d.specialty, d.shift1, d.shift2, d.location);
    fclose(f);
    printf("\n***********************************\n");
    printf("* Doctor Registered Successfully! *\n");
    printf("***********************************\n\n");

}

void doctorLogin(void)
{
    Doctor input, logged;
    printf("Enter username: "); scanf("%49s", input.username);
    printf("Enter password: "); scanf("%49s", input.password);

    FILE *f = fopen("doctors.txt", "r");
    if (!f)
    {
        printf("File error!\n");
        return;
    }
    int ok = 0;
    while (nextDoctor(f, &logged))
    {
        if (strcmp(input.username, logged.username) == 0 &&
            strcmp(input.password, logged.password) == 0)
            {
                 ok = 1;
                 break;
            }
    }
    fclose(f);

    if (!ok)
    {
        printf("Invalid username or password!\n");
        return;
    }
    printf("\nLogin successful! Welcome, %s\n", logged.name);
    doctorMenu(&logged);
}

void doctorForgetPassword(void)
{
    char uname[50], mobile[20], newpass[50];
    printf("Enter username: "); scanf("%49s", uname);
    printf("Enter mobile: ");   scanf("%19s", mobile);

    FILE *f = fopen("doctors.txt", "r");
    FILE *t = fopen("temp_doc.txt", "w");
    if (!f || !t)
    {
        printf("File error!\n");
        if(f)fclose(f); if(t)fclose(t);
        return;
    }

    Doctor d; int found = 0;
    while (nextDoctor(f, &d))
    {
        if (strcmp(d.username, uname) == 0 && strcmp(d.mobile, mobile) == 0)
        {
            printf("Enter new password: "); scanf("%49s", newpass);
            fprintf(t, "%s,%s,%s,%s,%s,%s,%s,%s,%s,%s\n",
                    d.username, newpass, d.mobile, d.nid, d.education,
                    d.name, d.specialty, d.shift1, d.shift2, d.location);
            found = 1;
        }
        else
        {
            fprintf(t, "%s,%s,%s,%s,%s,%s,%s,%s,%s,%s\n",
                    d.username, d.password, d.mobile, d.nid, d.education,
                    d.name, d.specialty, d.shift1, d.shift2, d.location);
        }
    }
    fclose(f); fclose(t);
    remove("doctors.txt");
    rename("temp_doc.txt", "doctors.txt");

    if (found)
        {
        printf("\n**********************************\n");
        printf("* Password changed successfully! *\n");
        printf("**********************************\n");
        }
    else
    {
        printf("Username or mobile not found!\n");
    }
}

void doctorMenu(Doctor *logged)
{
    int choice;
    while (1)
    {
        printf("\n--- Doctor Menu (%s) ---\n", logged->username);
        printf("1) View My Appointments\n");
        printf("2) Reschedule Appointment\n");
        printf("3) Add Patient Reminder\n");
        printf("0) Logout\n");
        printf("Enter choice: ");
        if (scanf("%d", &choice) != 1)
        {
            flush_input();
            continue;
        }
        getchar();

        if      (choice == 1) doctorViewMyAppointments(logged);
        else if (choice == 2) doctorReschedule(logged);
        else if (choice == 3) doctorAddReminder(logged);
        else if (choice == 0)
        return;
        else printf("Invalid choice!\n");
    }
}

void doctorViewMyAppointments(Doctor *logged)
{
    FILE *f = fopen("appointments.txt", "r");
    if (!f)
    {
        printf("No appointments yet.\n");
        return;
    }
    Appointment a;
    int any = 0,
    serial = 0;
    printf("\n--- Appointments for %s ---\n", logged->username);
    while (fscanf(f, "%49[^,],%49[^,],%19[^,],%d\n",
                  a.doctorUsername, a.patientUsername, a.date, &a.shift) == 4)
    {
        if (strcmp(a.doctorUsername, logged->username) == 0)
        {
            any = 1; serial++;
            printf("%d) Patient: %s | Date: %s | Shift: %d\n", serial, a.patientUsername, a.date, a.shift);
        }
    }
    fclose(f);
    if (!any)
    printf("\nNo appointments found.\n");
}

void doctorReschedule(Doctor *logged)
{
    char patientU[50], oldDate[20], newDate[20];
    int oldShift, newShift;
    printf("Enter patient username to reschedule: "); scanf("%49s", patientU);
    printf("Enter current date (DD-MM-YYYY): ");      scanf("%19s", oldDate);
    printf("Enter current shift (1/2): ");            scanf("%d", &oldShift);
    printf("Enter new date (DD-MM-YYYY): ");          scanf("%19s", newDate);
    printf("Enter new shift (1/2): ");                scanf("%d", &newShift);

    if (newShift != 1 && newShift != 2)
    {
        printf("Invalid new shift!\n");
        return;
    }
    int cap = countPatientsInShiftOnDate(logged->username, newShift, newDate);
    if (cap >= SHIFT_LIMIT)
    {
        printf("New shift is full. Choose another.\n");
        return;
    }

    FILE *f = fopen("appointments.txt", "r");
    FILE *t = fopen("temp_ap.txt", "w");
    if (!f || !t)
    {
        printf("File error!\n"); if(f)fclose(f); if(t)fclose(t);
        return;
    }

    Appointment a; int changed = 0;
    while (fscanf(f, "%49[^,],%49[^,],%19[^,],%d\n",
                  a.doctorUsername, a.patientUsername, a.date, &a.shift) == 4)
                 {
                    if (!changed && strcmp(a.doctorUsername, logged->username) == 0 &&
            strcmp(a.patientUsername, patientU) == 0 &&
            strcmp(a.date, oldDate) == 0 && a.shift == oldShift)
            {
                fprintf(t, "%s,%s,%s,%d\n", logged->username, patientU, newDate, newShift);
            changed = 1;
            FILE *r = fopen("reminders.txt", "a");
            if (r)
            {
                fprintf(r, "Reminder for %s: Your appointment with %s moved to %s (Shift %d)\n",
                             patientU, logged->username, newDate, newShift); fclose(r);
            }
            }
        else
        {
            fprintf(t, "%s,%s,%s,%d\n", a.doctorUsername, a.patientUsername, a.date, a.shift);
        }
                 }
    fclose(f); fclose(t);
    remove("appointments.txt");
    rename("temp_ap.txt", "appointments.txt");
    if (changed) { printf("\nRescheduled successfully and reminder sent.\n"); thankYouPopup(); }
    else printf("Matching appointment not found.\n");
}

void doctorAddReminder(Doctor *logged)
{
    char patientU[50], msg[120];
    printf("Enter patient username: "); scanf("%49s", patientU);
    printf("Enter short message: ");    scanf(" %119[^\n]", msg);

    FILE *r = fopen("reminders.txt", "a");
    if (!r)
    {
        printf("File error!\n");
        return;
    }
    fprintf(r, "Reminder for %s: From %s -> %s\n", patientU, logged->username, msg);
    fclose(r);
    printf("Reminder added for %s.\n", patientU);
    thankYouPopup();
}

void adminPortal(void)
{
    int choice;
    while (1)
    {
        printf("\n--- Hospital Admin Portal ---\n");
        printf("1) Login\n");
        printf("2) New Admin? Register Now!\n");
        printf("3) Forget Password\n");
        printf("0) Back\n");
        printf("Enter choice: ");
        if (scanf("%d", &choice) != 1)
        {
            flush_input();
            continue;
        }
        getchar();

        if      (choice == 1) adminLogin();
        else if (choice == 2) adminRegister();
        else if (choice == 3) adminForgetPassword();
        else if (choice == 0)
        return;
        else
        printf("Invalid choice!\n");
    }
}

void adminRegister(void)
{
    Admin a;
    printf("Choose admin username: ");   scanf("%49s", a.username);
    printf("Enter mobile: ");            scanf("%19s", a.mobile);
    printf("Enter password: ");          scanf("%49s", a.password);
    printf("Enter full name: ");         scanf(" %49[^\n]", a.fullName);
    printf("Enter NID: ");               scanf("%19s", a.nid);
    printf("Enter hospital name: ");     scanf(" %79[^\n]", a.hospitalName);
    printf("Enter hospital location: "); scanf(" %79[^\n]", a.hospitalLocation);

    FILE *f = fopen("admins.txt", "r");
    if (!f)
    {
        printf("File error!\n");
        return;
    }
    Admin tmp;
    while (nextAdmin(f, &tmp))
    {
        if (strcmp(tmp.username, a.username) == 0)
        {
            printf("Admin username already exists!\n"); fclose(f);
            return;
        }
    }
    fclose(f);

    f = fopen("admins.txt", "a");
    if (!f)
    {
        printf("File error!\n");
        return;
    }
    fprintf(f, "%s,%s,%s,%s,%s,%s,%s\n",
            a.username, a.mobile, a.password, a.fullName, a.nid, a.hospitalName, a.hospitalLocation);
    fclose(f);
    printf("\n**********************************\n");
    printf("* Admin registered successfully! *\n");
    printf("**********************************\n");
}

void adminForgetPassword(void)
{
    char uname[50], mobile[20], newpass[50];
    printf("Enter admin username: "); scanf("%49s", uname);
    printf("Enter mobile: ");         scanf("%19s", mobile);

    FILE *f = fopen("admins.txt", "r");
    FILE *t = fopen("temp_admin.txt", "w");
    if (!f || !t)
    {
        printf("File error!\n"); if(f)fclose(f); if(t)fclose(t);
        return;
    }

    Admin a; int found = 0;
    while (nextAdmin(f, &a))
    {
        if (strcmp(a.username, uname) == 0 && strcmp(a.mobile, mobile) == 0) {
            printf("Enter new password: "); scanf("%49s", newpass);
            fprintf(t, "%s,%s,%s,%s,%s,%s,%s\n",
                    a.username, a.mobile, newpass, a.fullName, a.nid, a.hospitalName, a.hospitalLocation);
            found = 1;
        }
        else
        {
            fprintf(t, "%s,%s,%s,%s,%s,%s,%s\n",
                    a.username, a.mobile, a.password, a.fullName, a.nid, a.hospitalName, a.hospitalLocation);
        }
    }
    fclose(f); fclose(t);
    remove("admins.txt"); rename("temp_admin.txt", "admins.txt");
    if (found)
        {
        printf("\n***************************\n");
        printf("* Admin password changed. *\n");
        printf("***************************\n");
        }
    else
    {
        printf("Admin not found.\n");
    }
}

void adminLogin(void)
{
    char uname[50], pass[50];
    printf("Enter admin username: "); scanf("%49s", uname);
    printf("Enter password: ");       scanf("%49s", pass);

    FILE *f = fopen("admins.txt", "r");
    if (!f)
    {
        printf("File error!\n");
        return;
    }
    Admin a; int ok = 0;
    while (nextAdmin(f, &a))
    {
        if (strcmp(a.username, uname) == 0 && strcmp(a.password, pass) == 0) { ok = 1; break; }
    }
    fclose(f);
    if (!ok) { printf("Invalid admin credentials!\n"); return; }
    printf("\nAdmin login successful! Welcome, %s (%s)\n", a.fullName, a.hospitalName);
    adminMenu(uname);
}

void adminMenu(char *adminUser)
{
    int choice;
    while (1)
    {
        printf("\n--- Admin Menu (%s) ---\n", adminUser);
        printf("1) View Appointments\n");
        printf("2) Edit Appointment\n");
        printf("3) Manage Doctors (Add/Delete)\n");
        printf("0) Logout\n");
        printf("Enter choice: ");
        if (scanf("%d", &choice) != 1)
        {
            flush_input();
            continue;
        }
        getchar();

        if      (choice == 1) adminViewAppointments();
        else if (choice == 2) adminEditAppointment();
        else if (choice == 3) adminManageDoctors();
        else if (choice == 0)
        return;
        else printf("Invalid choice!\n");
    }
}

void adminViewAppointments(void)
{
    FILE *f = fopen("appointments.txt", "r");
    if (!f)
    {
        printf("\nNo appointments file.\n");
        return;
    }
    Appointment a; int i = 0;
    printf("\n--- All Appointments ---\n");
    while (fscanf(f, "%49[^,],%49[^,],%19[^,],%d\n",
                  a.doctorUsername, a.patientUsername, a.date, &a.shift) == 4)
        {
            i++;
        printf("%d) Doctor: %s | Patient: %s | Date: %s | Shift: %d\n",
               i, a.doctorUsername, a.patientUsername, a.date, a.shift);
        }
    fclose(f);
    if (i == 0)
    printf("No appointments found.\n");
}

void adminEditAppointment(void)
{
    char docU[50], patU[50], oldDate[20], newDate[20];
    int oldShift, newShift;
    printf("Enter doctor username: ");  scanf("%49s", docU);
    printf("Enter patient username: "); scanf("%49s", patU);
    printf("Enter current date (DD-MM-YYYY): "); scanf("%19s", oldDate);
    printf("Enter current shift (1/2): "); scanf("%d", &oldShift);
    printf("Enter new date (DD-MM-YYYY): "); scanf("%19s", newDate);
    printf("Enter new shift (1/2): "); scanf("%d", &newShift);

    if (newShift != 1 && newShift != 2)
    {
        printf("Invalid new shift.\n");
        return;
    }
    int cap = countPatientsInShiftOnDate(docU, newShift, newDate);
    if (cap >= SHIFT_LIMIT)
    {
        printf("New slot full. Pick different.\n"); return;
    }

    FILE *f = fopen("appointments.txt", "r");
    FILE *t = fopen("temp_ap.txt", "w");
    if (!f || !t)
    {
        printf("File error!\n"); if(f)fclose(f); if(t)fclose(t); return;
    }

    Appointment a; int changed = 0;
    while (fscanf(f, "%49[^,],%49[^,],%19[^,],%d\n",
                  a.doctorUsername, a.patientUsername, a.date, &a.shift) == 4)
      {
        if (!changed && strcmp(a.doctorUsername, docU) == 0 &&
            strcmp(a.patientUsername, patU) == 0 &&
            strcmp(a.date, oldDate) == 0 &&
            a.shift == oldShift)
          {
            fprintf(t, "%s,%s,%s,%d\n", docU, patU, newDate, newShift);
            changed = 1;
          }
        else
        {
            fprintf(t, "%s,%s,%s,%d\n", a.doctorUsername, a.patientUsername, a.date, a.shift);
        }
      }
    fclose(f); fclose(t);
    remove("appointments.txt");
    rename("temp_ap.txt", "appointments.txt");
    if (changed) { printf("Appointment updated.\n"); thankYouPopup(); }
    else printf("Appointment not found.\n");
}

void adminManageDoctors(void)
{
    int ch;
    while (1)
    {
        printf("\n--- Manage Doctors ---\n");
        printf("1) Add Doctor\n");
        printf("2) Delete Doctor\n");
        printf("0) Back\n");
        printf("Enter choice: ");
        if (scanf("%d", &ch) != 1) { flush_input(); continue; }
        getchar();
        if      (ch == 1) adminAddDoctor();
        else if (ch == 2) adminDeleteDoctor();
        else if (ch == 0) return;
        else printf("Invalid choice!\n");
    }
}

void adminAddDoctor(void)
{
    Doctor d;
    printf("Enter username: ");   scanf("%49s", d.username);
    printf("Enter password: ");   scanf("%49s", d.password);
    printf("Enter mobile: ");     scanf("%19s", d.mobile);
    printf("Enter NID: ");        scanf("%19s", d.nid);
    printf("Enter education: ");  scanf(" %79[^\n]", d.education);
    printf("Enter name: ");       scanf(" %49[^\n]", d.name);
    printf("Enter specialty: ");  scanf(" %49[^\n]", d.specialty);
    printf("Enter shift1: ");     scanf(" %19[^\n]", d.shift1);
    printf("Enter shift2: ");     scanf(" %19[^\n]", d.shift2);
    printf("Enter location: ");   scanf(" %49[^\n]", d.location);

    FILE *f = fopen("doctors.txt", "r");
    if (!f) { printf("File error!\n"); return; }
    Doctor tmp; int exists = 0;
    while (nextDoctor(f, &tmp)) {
        if (strcmp(tmp.username, d.username) == 0) { exists = 1; break; }
    }
    fclose(f);
    if (exists) { printf("Doctor username exists!\n"); return; }

    f = fopen("doctors.txt", "a");
    if (!f) { printf("File error!\n"); return; }
    fprintf(f, "%s,%s,%s,%s,%s,%s,%s,%s,%s,%s\n",
            d.username, d.password, d.mobile, d.nid, d.education,
            d.name, d.specialty, d.shift1, d.shift2, d.location);
    fclose(f);
    printf("Doctor added.\n");
    thankYouPopup();
}

void adminDeleteDoctor(void)
{
    char uname[50];
    printf("Enter doctor username to delete: "); scanf("%49s", uname);

    FILE *f = fopen("doctors.txt", "r");
    FILE *t = fopen("temp_doc.txt", "w");
    if (!f || !t)
    {
         printf("File error!\n"); if(f)fclose(f); if(t)fclose(t);
         return;
    }

    Doctor d; int found = 0;
    while (nextDoctor(f, &d)) {
        if (strcmp(d.username, uname) == 0) { found = 1; continue; }
        fprintf(t, "%s,%s,%s,%s,%s,%s,%s,%s,%s,%s\n",
                d.username, d.password, d.mobile, d.nid, d.education,
                d.name, d.specialty, d.shift1, d.shift2, d.location);
    }
    fclose(f); fclose(t);
    remove("doctors.txt"); rename("temp_doc.txt", "doctors.txt");
    if (found) { printf("Doctor deleted.\n"); thankYouPopup(); }
    else printf("Doctor not found.\n");
}

void flush_input(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

