/**
 * @file        test.cpp
 * @author      Austin Glaser <glaserac@colorado.edu>
 * @brief       Test code for opencv image capture
 */

/* --- INCLUDES ----------------------------------------------------------- */

// Standard
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <queue>

// Opencv
#include <opencv.hpp>

// Boost
#include <boost/thread/thread.hpp>
#include <boost/interprocess/sync/interprocess_semaphore.hpp>

/* --- NAMESPACES --------------------------------------------------------- */

using namespace std;
using namespace cv;
using namespace boost;
using namespace boost::interprocess;

/* --- DEFINES ------------------------------------------------------------ */

#define TIME_MS(tv) ((((tv).tv_sec)*1000.0) + (((tv).tv_usec)/1000.0))

/* --- DATATYPES ---------------------------------------------------------- */

/**
 * @brief   Struct for options values and flags
 */
typedef struct {
    uint32_t n_frames;          /**< Number of frames to capture */
    char fmt[8];                /**< File format for images */
    bool save;                  /**< Whether or not to save capture frames to the disk */
    bool verbose;               /**< Whether to print extra info */
    bool display;               /**< Whether or not to display the image onscreen */
} OPTIONS_t;

/* --- LOCAL FUNCTION PROTOTYPES ------------------------------------------ */

/**
 * @brief   Prints usage instructions and exits
 * @param[in]   call:       The string which was used to call the program
 * @param[in]   err:        Error code to return to system
 */
static void usage(char * call, int32_t err);

/**
 * @brief   Parses command line arguments, sets flags and values within the options struct
 * @param[in]   argc:       The number of arguments given, including the calling string
 * @param[in]   argv:       An array of options strings
 * @param[out]  options:    A struct containing various options values and flags, set appropriately
 *                          from the provided input
 * @return:                 0 upon success, -1 upon finding malformatted input
 */
static uint32_t parse_args(int32_t argc, char ** argv, OPTIONS_t * options);

/* --- LOCAL FUNCTION DEFINITIONS ----------------------------------------- */

/**
 * @brief   Captures images from an attached webcam and times framerate
 * @note    Currently uses the 'default' webcam device (/dev/video0)
 * @param[in]   argc:   The number of command line arguments. From the system
 * @param[in]   argv:   An array of strings from the command line. From the system
 * @return:             0 upon success, non-zero upon an error
 */
int32_t main(int32_t argc, char ** argv)
{
    uint32_t i;
    OPTIONS_t options;

    char title[128];
    Mat frame;

    struct timeval tv;
    double start_ms, last_ms, now_ms, diff_ms, rel_ms, diff_ms_avg;

    // Parse command-line arguments. Print usage and returns if an error is found
    if (parse_args(argc, argv, &options)) usage(argv[0], -1);

    // Open capture stream
    VideoCapture cap(0);
    if (!cap.isOpened()) return 1;

    // Set capture properties
    cap.set(CV_CAP_PROP_FRAME_WIDTH,320);
    cap.set(CV_CAP_PROP_FRAME_HEIGHT,240);
    system("v4l2-ctl -p10");           // Currently DOESN'T set framerate. It tries to tell us it has though

    // Make image directory and clean it out if we're saving this run
    if (options.save) {
        system("mkdir -p images");
        system("rm -rf images/*");
    }

    // Get start information for timing
    gettimeofday(&tv, NULL);
    start_ms = TIME_MS(tv);
    last_ms = start_ms;

    diff_ms_avg = 0;

    namedWindow("Image");
    for (i = 0; i < options.n_frames; i++) {
        // Grab frame
        cap.read(frame);

        // Get timing info
        gettimeofday(&tv, NULL);
        now_ms = TIME_MS(tv);
        rel_ms = now_ms - start_ms; // ms from start
        diff_ms = now_ms - last_ms; // ms from last image

        // Print out frame info if verbose mode is set
        if (options.verbose) printf("[%4d] Relative: %05.lf\tDiff: %05.lf\n", i,  rel_ms, diff_ms);

        last_ms = now_ms;

        // Save image if save mode is set
        if (options.save) {
            // Filename includes frame number, time from start, and time from last frame
            sprintf(title, "images/%05d.%05.lf.%05.lf.%s", i, rel_ms, diff_ms, options.fmt);
            if (!imwrite(title, frame)) return 2;
        }

        if (options.display) {
            imshow(title, frame);
        }

        // Accumulate for average frame timing
        diff_ms_avg += diff_ms;
    }

    // Calculate and print average interval and overall framerate
    diff_ms_avg /= options.n_frames;
    printf("\nAverage: %05.lf\t(%05.lf FPS)\n\n", diff_ms_avg, 1000/diff_ms_avg);

    cap.release();
    destroyAllWindows();

    return 0;
}

static void usage(char * call, int32_t err)
{
    // Print usage info
    printf("Usage:\t%s -n<n_frames> [OPTIONS]\n"
           "Options available:\n"
           "\t-v\t\tVerbose mode (default: off)\n"
           "\t-s\t\tSaves frames under images/ directory (default: off)\n"
           "\t-d\t\tDisplays images on the screen (default: off)\n"
           "\t-f<fmt>\t\tSets format to the specified value (default: jpg)\n"
           "\t-h\t\tPrints this message\n", call);

    // Exit
    exit(err);
}

static uint32_t parse_args(int32_t argc, char ** argv, OPTIONS_t * options)
{
    int32_t i;

    // Set defaults
    options->n_frames   = 0;
    strcpy(options->fmt, "jpg");
    options->save       = false;
    options->verbose    = false;
    options->display    = false;

    // Loops through all strings
    for (i = 1; i < argc; i++) {
        // If it's a flag, deal with it accordingly
        if (argv[i][0] == '-') switch (argv[i][1]) {
            case 'v':       // Verbose flag
                options->verbose = true;
                break;

            case 's':       // Save flag
                options->save = true;
                break;

            case 'f':       // Format flag. Invalid formats sorted by opencv
                strcpy(options->fmt, &(argv[i][2]));
                break;

            case 'n':       // Number of frames
                if (sscanf(&(argv[i][2]), "%u", &(options->n_frames)) != 1) return -1;
                break;

            case 'd':
                options->display = true;
                break;

            case 'h':       // Help flag
                usage(argv[0], 0);
                break;
                
            default:        // Unrecognized flag
                return -1;
                break;
        }
        else return -1;
    }

    // n_frames is required and must be nonzero
    if (options->n_frames)   return 0;
    else                    return -1;
}
