// Hippotronics LED lamp web requests functions

// Prototypes
void setup_web_paths();
void sendHtml(AsyncWebServerRequest *request, String title, String body);
void report_status(AsyncWebServerRequest *request);
void report_status_json(AsyncWebServerRequest *request);
