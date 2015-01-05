// gcc -Wall peb-rss.c -o peb-rss.exe %GLIB_FLAGS% -lcurldll
#include <stdio.h>
#include <glib.h>
#include <curl/curl.h>
#include <time.h>
#include <windows.h>
#include <shellapi.h>
 
/*********** zmienne curl *******************/
char *dramatHQ = "http://peb.pl/high-quality-dramat-obyczajowe/";
char *animeHQ = "http://peb.pl/high-quality-anime/";
char *scifiHQ = "http://peb.pl/high-quality-fantasy-sci-fi/";
char *animowaneHQ = "http://peb.pl/high-quality-filmy-animowane/";
char *dokumentHQ = "http://peb.pl/high-quality-filmy-dokumentalne/";
char *horrorHQ = "http://peb.pl/high-quality-horror-thriller/";
char *komediaHQ = "http://peb.pl/high-quality-komedie-komedie-romantyczne/";
char *sensacjaHQ = "http://peb.pl/high-quality-sensacyjne-przygodowe/";
 
char *dramatURL = "high-quality-dramat-obyczajowe";
char *animeURL = "high-quality-anime";
char *scifiURL = "high-quality-fantasy-sci-fi";
char *animowaneURL = "high-quality-filmy-animowane";
char *dokumentURL = "high-quality-filmy-dokumentalne";
char *horrorURL = "high-quality-horror-thriller";
char *komediaURL = "high-quality-komedie-komedie-romantyczne";
char *sensacjaURL = "high-quality-sensacyjne-przygodowe";
 
char tabTitles[20][200];
char tabUrls[20][200];
 
struct memoryStruct
{
    char *memory;
    size_t size;
};
 
static size_t write_mem_callback(void *content, 
    size_t size, size_t nmemb, void *userdata)
{
    size_t realsize = size * nmemb;
    struct memoryStruct *mem = (struct memoryStruct *)userdata;
 
    mem->memory = realloc(mem->memory, mem->size + realsize + 1);
    if(mem->memory == NULL)
    {
        fprintf(stderr, "realloc returned NULL - out of memory\n");
        return 0;
    }
    memcpy(&(mem->memory[mem->size]), content, realsize);
    mem->size = mem->size + realsize;
    mem->memory[mem->size] = 0;
 
    return realsize;
}
 
void get_title(char *input_text)
{
  /*
    przetwarzamy kazdy znacznik html(rozpoczynajacy sie nawiasem <), 
    nastepnie szukamy w  kazdym html_token wyrazenia thread_title, 
    usuwamy poczatek pozostawiajac tytul
  */
  int i = 0;
  char *html_token, *html_delimiter = "<";
  char *title_token, *title;
   
  html_token = strtok(input_text, html_delimiter);
  while(html_token != NULL && i < 20)
  {
    html_token = strtok(NULL, html_delimiter);
    if(html_token != NULL) // sprawdzamy bo strstr() sie wysypie dla NULL
    {
      title_token = strstr(html_token, "thread_title");
      if(title_token != NULL)
      {
        title = strstr(title_token, ">");
        if(title != NULL)
        {
          // inverted scanset, so scanf keeps taking in values 
          // until it encounters a '\n'
          // +1 usuwamy nawias z pocztku
          sscanf(title + 1, "%199[^\n]", tabTitles[i]); 
          tabTitles[i][200] = '\0';
          ++i; 
        }
      } 
    }
  }
}
 
void get_url(char *input_text, const char *forum_url)
{
    char *html_token, *html_delimiter = "<";
  char *http_token, *url_token, *url_token_end;
  char link_chunk[300];
    int i = 0;
   
  html_token = strtok(input_text, html_delimiter);
  while(html_token != NULL && i < 20)
  {
    html_token = strtok(NULL, html_delimiter);
    if(html_token != NULL) // sprawdzamy bo strstr() sie wysypie dla NULL
    {
        http_token = strstr(html_token, "http");
      if(http_token != NULL)
      {
        url_token = strstr(http_token, forum_url);
        if(url_token != NULL)
            {
            url_token_end = strstr(url_token, ".html\" id=");
          if(url_token_end != NULL)
          {
            memset(link_chunk, '\0', sizeof(link_chunk));
            strncpy(link_chunk, url_token, url_token_end - url_token);
            //link_chunk[url_token_end-url_token+1] = '\0'; // czy to potrzebne?
            gchar *link = g_strconcat("http://peb.pl/", link_chunk, ".html", NULL);
            sscanf(link, "%199[^\n]", tabUrls[i]); 
            tabUrls[i][200] = '\0';
            ++i;
            g_free(link);
          } 
        }
      } 
    }
  }
}
 
void run_curl(const char *forum, const char *forum_url)
{
    gchar *htmlTitle, *htmlUrl;
    CURL *handle;
  CURLcode res;
  struct memoryStruct chunk;
   
  chunk.memory = malloc(1);
  chunk.size = 0;
   
  curl_global_init(CURL_GLOBAL_ALL);
  handle = curl_easy_init();
  curl_easy_setopt(handle, CURLOPT_URL, forum);
  curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, write_mem_callback);
  curl_easy_setopt(handle, CURLOPT_WRITEDATA, (void *)&chunk);
  curl_easy_setopt(handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");
  res = curl_easy_perform(handle);
   
  if(res != CURLE_OK)
    fprintf(stderr, "error: %s\n", curl_easy_strerror(res));
  else
  {
    htmlTitle = g_convert(chunk.memory, strlen(chunk.memory), 
                                                "UTF-8", "ISO-8859-2", NULL, NULL, NULL);
    htmlUrl = g_strdup(htmlTitle);
    get_title(htmlTitle);
        get_url(htmlUrl, forum_url);
    if(htmlTitle)
        g_free(htmlTitle);
    if(htmlUrl)
        g_free(htmlUrl);
  }
   
  curl_easy_cleanup(handle);
  if(chunk.memory)
    free(chunk.memory);
  curl_global_cleanup();
}
 
int main(void)
{
    FILE *fp;
    int i, j;
 
    time_t rawtime;
  struct tm * timeinfo;
     
    time ( &rawtime );
  timeinfo = localtime ( &rawtime );
     
    printf("Pobieranie danych.");
 
    fp = fopen("peb.html", "w");
    fprintf(fp, "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">\n"
        "<html xmlns=\"http://www.w3.org/1999/xhtml\" xml:lang=\"pl\">\n"
        "<head>\n"
        "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" />"
        "<title>Peb rss</title>\n"
        "<style type=\"text/css\">\n"
        "body{color: #8c909a; background: #383937; font-family: sans-serif; font-size: 0.9em;}\n"
        "h2{color: #ffffff;}\n"
        "a:link{color: #fbaf32; text-decoration: underline;}\n"
        "a:hover{color: #CBB585; text-decoration: none;}\n"
        "a:visited{color: #CBB585; text-decoration: none;}\n"
        "</style>\n"
        "</head>\n"
        "<body>\n");
 
    fprintf(fp, "%s<br />", asctime(timeinfo));
     
    for(i = 0; i < 8; ++i)
  {
    printf(".");
    switch(i)
    {
        case 0:
            fprintf(fp, "<h2>dramat-obyczajowe</h2>\n");
            run_curl(dramatHQ, dramatURL);
            for(j = 0; j < 20; ++j)
            {
                fprintf(fp, "<a href=\"%s\">", &tabUrls[j][200]);
                fprintf(fp, "%s</a><br />", &tabTitles[j][200]);    
            }
            break;
        case 1:
            fprintf(fp, "<h2>anime</h2>\n");
            run_curl(animeHQ, animeURL);
            for(j = 0; j < 20; ++j)
            {
                fprintf(fp, "<a href=\"%s\">", &tabUrls[j][200]);
                fprintf(fp, "%s</a><br />", &tabTitles[j][200]);    
            }
            break;
        case 2:
            fprintf(fp, "<h2>fantasy-sci-fi</h2>\n");
            run_curl(scifiHQ, scifiURL);
            for(j = 0; j < 20; ++j)
            {
                fprintf(fp, "<a href=\"%s\">", &tabUrls[j][200]);
                fprintf(fp, "%s</a><br />", &tabTitles[j][200]);    
            }
            break;
        case 3:
            fprintf(fp, "<h2>filmy-animowane</h2>\n");
            run_curl(animowaneHQ, animowaneURL);
            for(j = 0; j < 20; ++j)
            {
                fprintf(fp, "<a href=\"%s\">", &tabUrls[j][200]);
                fprintf(fp, "%s</a><br />", &tabTitles[j][200]);    
            }
            break;
        case 4:
            fprintf(fp, "<h2>filmy-dokumentalne</h2>\n");
            run_curl(dokumentHQ, dokumentURL);
            for(j = 0; j < 20; ++j)
            {
                fprintf(fp, "<a href=\"%s\">", &tabUrls[j][200]);
                fprintf(fp, "%s</a><br />", &tabTitles[j][200]);    
            }
            break;
        case 5:
            fprintf(fp, "<h2>horror-thriller</h2>\n");
            run_curl(horrorHQ, horrorURL);
            for(j = 0; j < 20; ++j)
            {
                fprintf(fp, "<a href=\"%s\">", &tabUrls[j][200]);
                fprintf(fp, "%s</a><br />", &tabTitles[j][200]);    
            }
            break;
        case 6:
            fprintf(fp, "<h2>komedie-komedie-romantyczne</h2>\n");
            run_curl(komediaHQ, komediaURL);
            for(j = 0; j < 20; ++j)
            {
                fprintf(fp, "<a href=\"%s\">", &tabUrls[j][200]);
                fprintf(fp, "%s</a><br />", &tabTitles[j][200]);    
            }
            break;
        default:
            fprintf(fp, "<h2>sensacyjne-przygodowe</h2>\n");
            run_curl(sensacjaHQ, sensacjaURL);
            for(j = 0; j < 20; ++j)
            {
                fprintf(fp, "<a href=\"%s\">", &tabUrls[j][200]);
                fprintf(fp, "%s</a><br />", &tabTitles[j][200]);    
            }
    }
  }
  fprintf(fp, "</body>"
              "</html>");
  fclose(fp);
  ShellExecute(NULL, "open", "peb.html", NULL, NULL, SW_SHOWNORMAL);
  return 0;
}