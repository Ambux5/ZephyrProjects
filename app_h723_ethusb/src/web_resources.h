/**
 * @file web_resources.h
 * @brief Web server HTML resources with Bootstrap styling
 * 
 * Modern responsive HTML pages using Bootstrap CSS framework.
 * Uses C preprocessor macros to avoid code duplication while keeping
 * all HTML as static strings (no runtime allocation).
 */

#pragma once

/* Helper macros for string conversion */
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

/* Default version if not provided by build system */
#ifndef ZEPHYR_VERSION_STRING
#define ZEPHYR_VERSION_STRING unknown
#endif

/* ============================================================================
 * SHARED COMPONENTS - Used by all pages
 * ============================================================================ */

/**
 * @brief HTML header start - DOCTYPE, charset, meta tags, and CSS styles
 * (note: does not include </head><body> - those are added per page with title)
 */
#define HTML_HEAD_START \
    "<!DOCTYPE html><html><head>" \
    "<meta charset='UTF-8'><meta name='viewport' content='width=device-width,initial-scale=1'>" \
    "<style>" \
    "body{margin:0;font-family:Consolas,Monaco,'Courier New',monospace;background:#1e1e1e;color:#d4d4d4}" \
    ".navbar{background:#252526;padding:.5rem 1rem;color:#d4d4d4;display:flex;justify-content:space-between;align-items:center;border-bottom:1px solid #3c3c3c}" \
    ".navbar-brand{font-size:1.25rem;font-weight:700;color:#569cd6}" \
    ".nav{display:flex;list-style:none;margin:0;padding:0}" \
    ".nav a{color:#d4d4d4;text-decoration:none;padding:.5rem 1rem;display:block}" \
    ".nav a:hover{background:#2d2d30}" \
    ".container{max-width:1200px;margin:0 auto;padding:1rem}" \
    ".row{display:flex;gap:1rem;flex-wrap:wrap}" \
    ".col{flex:1;min-width:200px}" \
    ".card{border:1px solid #3c3c3c;background:#252526;border-radius:6px;padding:1rem;margin-bottom:1rem;box-shadow:0 0 10px rgba(0,0,0,.4)}" \
    ".card-header{font-weight:700;border-bottom:1px solid #3c3c3c;padding-bottom:.5rem;margin-bottom:.5rem;color:#569cd6}" \
    ".btn{display:inline-block;padding:.375rem .75rem;background:#0e639c;color:#ffffff;text-decoration:none;border-radius:4px;border:1px solid #1177bb;cursor:pointer}" \
    ".btn:hover{background:#1177bb}" \
    ".badge{display:inline-block;background:#6a9955;color:#ffffff;padding:.25rem .5rem;border-radius:4px;font-size:.85em}" \
    "h1,h2{color:#569cd6;margin-top:0}" \
    ".footer{text-align:center;padding:1rem;font-size:.875rem;color:#808080;border-top:1px solid #3c3c3c;margin-top:1rem}" \
    ".table{width:100%;border-collapse:collapse;margin-bottom:1.5rem;font-size:.95rem}" \
    ".table th{background:#2d2d30;color:#569cd6;text-align:left;padding:.6rem;border-bottom:2px solid #3c3c3c;font-weight:600}" \
    ".table td{padding:.6rem;border-bottom:1px solid #3c3c3c}" \
    ".table tr:nth-child(even){background:#252526}" \
    ".table tr:hover{background:#2a2d2e}" \
    ".table td:first-child{color:#9cdcfe;width:40%}" \
    ".table td:last-child{color:#ce9178;font-weight:500}" \
    ".form-group{margin-bottom:1rem}" \
    ".form-group label{display:block;margin-bottom:.5rem;color:#9cdcfe;font-weight:600}" \
    ".form-group input,.form-group select{width:100%;padding:.5rem;background:#1e1e1e;color:#d4d4d4;border:1px solid #3c3c3c;border-radius:4px;font-family:inherit}" \
    "</style>"

/**
 * @brief End of head and start of body
 */
#define HTML_HEAD_END \
    "</head><body>"\
     "<title>USB HTTP server demo</title>"

/**
 * @brief Shared navigation bar for all pages
 */
#define HTML_NAVBAR \
    "<div class='navbar'>" \
    "<div class='navbar-brand'>🚀 USB HTTP server demo</div>" \
    "<ul class='nav'>" \
    "<li><a href='/'>Home</a>" \
    "<li><a href='/status'>Status</a>" \
    "<li><a href='/info'>Info</a>" \
    "</ul>" \
    "</div>"

/**
 * @brief Shared page footer for all pages
 */
#define HTML_PAGE_FOOTER \
    "<div class='footer'>USB Web Demo - Zephyr RTOS</div>" \
    "</div></body></html>"

/* ============================================================================
 * PAGE CONTENT - Individual page body content (without structure)
 * ============================================================================ */

#define HTML_INDEX_CONTENT \
    "<div class='container'><h1>USB Connected Device</h1><div class='row'>" \
    "<div class='col'><div class='card'><div class='card-header'>⚙️ RTOS</div>" \
    "<p><strong>System:</strong> Zephyr RTOS</p>" \
    "<p><strong>Interface:</strong> USB Network</p>" \
    "</div></div></div></div>"

#define HTML_STATUS_CONTENT \
    "<div class='container'><h2>System Status</h2>" \
    "<table class='table'><thead><tr><th>System</th><th>Status</th></tr></thead><tbody>" \
    "<tr><td>Network</td><td>192.0.2.1</td></tr>" \
    "<tr><td>Port</td><td>8080 (TCP)</td></tr>" \
    "</tbody></table></div>"

#define HTML_INFO_CONTENT \
    "<div class='container'><h2>Device Info</h2>" \
    "<table class='table'><thead><tr><th>Software</th><th>Version</th></tr></thead><tbody>" \
    "<tr><td>RTOS</td><td>Zephyr " TOSTRING(ZEPHYR_VERSION_STRING) "</td></tr>" \
    "</tbody></table></div>"


#define HTML_NOTFOUND_CONTENT \
    "<div class='container'><h1>404 - Not Found</h1>" \
    "<p>The requested page does not exist.</p>" \
    "<p><a href='/' class='btn'>Back to Home</a></p></div>"

/* ============================================================================
 * COMPLETE PAGES - Assembled from components
 * ============================================================================ */

/**
 * @brief HTML home page - assembled from components
 */
static const char *html_index_page =
    HTML_HEAD_START
    HTML_HEAD_END
    HTML_NAVBAR
    HTML_INDEX_CONTENT
    HTML_PAGE_FOOTER;

/**
 * @brief Status page - assembled from components
 */
static const char *html_status_page =
    HTML_HEAD_START
    HTML_HEAD_END
    HTML_NAVBAR
    HTML_STATUS_CONTENT
    HTML_PAGE_FOOTER;

/**
 * @brief Info page - assembled from components
 */
static const char *html_info_page =
    HTML_HEAD_START
    HTML_HEAD_END
    HTML_NAVBAR
    HTML_INFO_CONTENT
    HTML_PAGE_FOOTER;

/**
 * @brief Configuration page - empty, not used
 */
static const char *html_config_page =
    HTML_HEAD_START
    HTML_HEAD_END
    HTML_NAVBAR
    "<div class='container'><h1>Not Available</h1></div>"
    HTML_PAGE_FOOTER;

/**
 * @brief 404 Not Found page - assembled from components
 */
static const char *html_not_found =
    HTML_HEAD_START
    HTML_HEAD_END
    HTML_NAVBAR
    HTML_NOTFOUND_CONTENT
    HTML_PAGE_FOOTER;
