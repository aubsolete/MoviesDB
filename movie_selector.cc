#include <iostream>
#include <iomanip>
#include <sqlite3.h>
#include <string>
using namespace std;

// Functions for introductory selection.
void watch_movie(sqlite3 *DB);
void delete_movie(sqlite3 *DB);
void add_movie(sqlite3 *DB);

// Functions for watch_movie() selection.
void watch_by_title(sqlite3 *DB);
void watch_by_genre(sqlite3 *DB);
void watch_by_favorites(sqlite3 *DB);
void watch_by_neverwatched(sqlite3 *DB);

// Functions for delete_movie() selection.
void delete_by_title(sqlite3 *DB);
void delete_by_genre(string &genre, sqlite3 *DB);

// Function for add_movie() selection.
void get_add_info(string &title, string &genre, string &neverwatched, string &favorite);

// Functions for sqlite3_exec().
static int normal_callback(void *DB, int command, char **arg, char **column);
static int check_callback(void *DB, int command, char **arg, char **column);


int main() {
    // Open database and check if successful.
    sqlite3 *DB;
    int successful_launch = sqlite3_open("MyMovies.db", &DB);
    if(successful_launch) {
        cout << "There was an error connecting to the database." << endl;
        return 0;
    }

    // Greet user and provide intro options.
    cout << "Welcome to your personal movie selector. What would you like to do?" << endl;
    char intro_selection = '\0';
    while(toupper(intro_selection) < 'A' || toupper(intro_selection) > 'D') {
        cout << "A. Add a movie" << endl;
        cout << "B. Delete a movie" << endl;
        cout << "C. Watch a movie" << endl;
        cout << "D. Exit" << endl;
        cout << "Answer (A-D) = ";
        cin >> intro_selection;
    }

    // Obtain user's answer, and appropriately handle it.
    if(toupper(intro_selection) == 'A') {
        add_movie(DB);
    } else if(toupper(intro_selection) == 'B') {
        delete_movie(DB);
    } else if(toupper(intro_selection) == 'C') {
        watch_movie(DB);
    } else if(toupper(intro_selection) == 'D') {
        cout << "Goodbye." << endl;
    } else {
        cout << "Sorry, an unexpected error occurred.";
    }

    // Close database and end program.
    sqlite3_close(DB);
    return 0;
}


// Normal_callback() is meant for general execution of commands to database. See like 76.
static int normal_callback(void *DB, int command, char **arg, char **column) {
	for(int i = 0; i < command; ++i) {
		cout << "\n" << (arg[i] ? arg[i] : "NULL") << endl;
	}
	return 0;
}


// Check_callback() is meant to handle the execution of a command to database that would return
// either true or false. This is to help determine if the user is adding a movie that already exists.
static int check_callback(void *DB, int command, char **arg, char **column) {
	for(int i = 0; i < command; ++i) {
        if(*arg[i] == '1')
            return 1;
	}
	return 0;
}


// Allow the user to add a movie using the "insert" command.
void add_movie(sqlite3 *DB) {
    string add = "insert into MyMovies (Title, Genre, Watched, Favorite) values ('";
    string exists = "select exists(select 1 from MyMovies where Title = '";
    string title = "", genre = "", neverwatched = "", favorite = "";

    const char *tbl[1];
    tbl[0] = exists.c_str();
    get_add_info(title, genre, neverwatched, favorite); // Get movie-to-add info from user.
    exists += (title + "');");
    tbl[0] = exists.c_str();
    int result = sqlite3_exec(DB, tbl[0], check_callback, NULL, NULL);
    
    // Check if movie is already in database; if not, add it.
    if(result == 0) {
        add += (title + "', '" + genre + "', " + neverwatched + ", '" + favorite + "');");
        tbl[0] = add.c_str();
        sqlite3_exec(DB, tbl[0], normal_callback, NULL, NULL);
    } else {
        cout << "That movie already exists! :)" << endl;
    }
}


// Allow the user to delete a movie using the "delete" command.
void delete_movie(sqlite3 *DB) {
    // Gather some helpful info from user. This could be useful if a user who is a parent wants to
    // delete all horror films from a child's movie list.
    cout << "What would you like to search by to delete?" << endl;
    char delete_selection = '\0';
    while(toupper(delete_selection) < 'A' || toupper(delete_selection) > 'E') {
        cout << "A. Title" << endl;
        cout << "B. Genre" << endl;
        cout << "C. Exit" << endl;
        cout << "Answer (A-C) = ";
        cin >> delete_selection;
    }

    // Handle the user's input with the appropiate sub-function.
    if(toupper(delete_selection) == 'A') {
        delete_by_title(DB);
    } else if(toupper(delete_selection) == 'B') {
        string genre = "";
        delete_by_genre(genre, DB);
        cout << "These are all of your movies of the genre " << genre << ". Now... " << endl;
        delete_by_title(DB);
    } else if(toupper(delete_selection) == 'C') {
        cout << "Goodbye." << endl;
    } else {
        cout << "Sorry, an unexpected error occurred.";
    }
}


// Allow the user to watch a movie, and to select a movie based on different criteria if unsure what to watch.
void watch_movie(sqlite3 *DB) {
    cout << "What would you like to search by to watch?" << endl;
    char watch_selection = 'N';
    while(toupper(watch_selection) < 'A' || toupper(watch_selection) > 'E') {
        cout << "A. Title" << endl;
        cout << "B. Genre" << endl;
        cout << "C. Favorites" << endl;
        cout << "D. Never Watched" << endl;
        cout << "E. Exit" << endl;
        cout << "Answer (A-E) = ";
        cin >> watch_selection;
    }

    // Based on the user's info, select the movie(s) to watch.
    if(toupper(watch_selection) == 'A') {
        watch_by_title(DB);
    } else if(toupper(watch_selection) == 'B') {
        watch_by_genre(DB);
    } else if(toupper(watch_selection) == 'C') {
        watch_by_favorites(DB);
    } else if(toupper(watch_selection) == 'D') {
        watch_by_neverwatched(DB);
    } else if(toupper(watch_selection) == 'E') {
        cout << "Goodbye." << endl;
    } else {
        cout << "Sorry, an unexpected error occurred.";
    }
}


void get_add_info(string &title, string &genre, string &neverwatched, string &favorite) {
    cout << "How many words (including standalone numbers) are in the title? For example, \"Deadpool 2\" would have 2 words." << endl;
    int wordcount = 0;
    cin >> wordcount;
    cout << "What's the title?" << endl;
    for(int i = 0; i < wordcount; ++i) {
        string build = "";
        cin >> build;
        title += build;
        if(i < (wordcount-1)) {
            title += " ";
        }
    }
    title[0] = toupper(title[0]);
    cout << "What's the genre?" << endl;
    cin >> genre;
    genre[0] = toupper(genre[0]);
    cout << "Have you watched it before?" << endl;
    cin >> neverwatched;
    if(toupper(neverwatched[0]) == 'Y') {
        neverwatched = "1";
    } else {
        neverwatched = "0";
    }
    cout << "Is it a favorite of yours?" << endl;
    cin >> favorite;
    favorite[0] = toupper(favorite[0]);
}


void watch_by_title(sqlite3 *DB) {
    string title, increment;
    const char *tbl[1];
    cout << "What is the title of the movie you wish to watch?" << endl;
    cin >> title;
    title[0] = toupper(title[0]);
    string find = ("select Title from MyMovies where Title = '" + title + "';");
    tbl[0] = find.c_str();
    cout << "The movie you'll be watching is... ";
    sqlite3_exec(DB, tbl[0], normal_callback, NULL, NULL);
    increment = ("update MyMovies set Watched = Watched + 1 where Title = '" + title + "';");
    tbl[0] = increment.c_str();
    sqlite3_exec(DB, tbl[0], normal_callback, NULL, NULL);
}


void watch_by_genre(sqlite3 *DB) {
    string genre;
    const char *tbl[1];
    cout << "What is the genre of the movie you wish to watch?" << endl;
    cin >> genre;
    genre[0] = toupper(genre[0]);
    string find_all = ("select Title from MyMovies where Genre = '" + genre + "';");
    tbl[0] = find_all.c_str();
    sqlite3_exec(DB, tbl[0], normal_callback, NULL, NULL);
    cout << "These are all of your movies of the genre " << genre << ". Now... " << endl;
    watch_by_title(DB);
}


void watch_by_favorites(sqlite3 *DB) {
    string favorites;
    const char *tbl[1];
    cout << "Is the movie you want to watch a favorite of yours? (Yes/No)" << endl;
    cin >> favorites;
    favorites[0] = toupper(favorites[0]);
    string find_all = ("select Title from MyMovies where Favorite = '" + favorites + "';");
    tbl[0] = find_all.c_str();
    sqlite3_exec(DB, tbl[0], normal_callback, NULL, NULL);
    cout << "These are all of your movies you declared " << favorites << " for Favoritism. Now... " << endl;
    watch_by_title(DB);
}


void watch_by_neverwatched(sqlite3 *DB) {
    string neverwatched, find_all = "";
    const char *tbl[1];
    cout << "Do you want to watch a movie you haven't seen before? (Yes/No)" << endl;
    cin >> neverwatched;
    neverwatched[0] = toupper(neverwatched[0]);
    if(neverwatched[0] == 'Y') {
        neverwatched = "0";
        find_all = ("select Title from MyMovies where Watched = '" + neverwatched + "';");
    } else {
        neverwatched = "0";
        find_all = ("select Title from MyMovies where Watched > '" + neverwatched + "';");
    }
    tbl[0] = find_all.c_str();
    sqlite3_exec(DB, tbl[0], normal_callback, NULL, NULL);
    cout << "These are the movies selected for you. Now... " << endl;
    watch_by_title(DB);
}


void delete_by_title(sqlite3 *DB) {
    const char *tbl[1];
    string delete_by = "delete from MyMovies where Title = '";
    string title = "";
    cout << "How many words (including standalone numbers) are in the title you wish to delete? For example, \"Deadpool 2\" would have 2 words." << endl;
    int wordcount = 0;
    cin >> wordcount;
    cout << "What's the title?" << endl;
    for(int i = 0; i < wordcount; ++i) {
        string build = "";
        cin >> build;
        title += build;
        if(i < (wordcount-1)) {
            title += " ";
        }
    }
    title[0] = toupper(title[0]);
    delete_by += (title + "';");
    tbl[0] = delete_by.c_str();
    sqlite3_exec(DB, tbl[0], normal_callback, NULL, NULL);
}


void delete_by_genre(string &genre, sqlite3 *DB) {
    const char *tbl[1];
    cout << "What is the genre of the movie you wish to delete?" << endl;
    cin >> genre;
    genre[0] = toupper(genre[0]);
    string find_all = ("select Title from MyMovies where Genre = '" + genre + "';");
    tbl[0] = find_all.c_str();
    sqlite3_exec(DB, tbl[0], normal_callback, NULL, NULL);
    cout << endl;
}
