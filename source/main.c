#include <3ds.h>
#include <curl/curl.h>
#include <stdio.h>
#include <string.h>

#define MAX_URL_LEN 512
#define MAX_PATH_LEN 768
#define MAX_FILENAME_LEN 256

const char *folders[] = {"/", "/3ds/", "/cias/", "/luma/plugins/"};
int folderCount = 4;

void drawFolderMenu(int selected) {
    consoleClear();
    printf("保存先ディレクトリを選んでください:\n\n");
    for (int i = 0; i < folderCount; i++) {
        printf((i == selected) ? " > %s\n" : "   %s\n", folders[i]);
    }
    printf("\nA: 決定 / B: 終了 / ↑↓: 選択");
}

Result showKeyboard(char *out, int size, const char *hint) {
    SwkbdState swkbd;
    swkbdInit(&swkbd, SWKBD_TYPE_NORMAL, 1, -1);
    swkbdSetHintText(&swkbd, hint);
    return swkbdInputText(&swkbd, out, size);
}

int confirmProceed(const char *msg) {
    consoleClear();
    printf("%s\n\nA: 続行 / B: キャンセル", msg);
    while (aptMainLoop()) {
        hidScanInput();
        u32 kDown = hidKeysDown();
        if (kDown & KEY_A) return 1;
        if (kDown & KEY_B) return 0;
        gspWaitForVBlank();
    }
    return 0;
}

int main(int argc, char **argv) {
    gfxInitDefault();
    consoleInit(GFX_TOP, NULL);

    int selected = 0;
    bool running = true;

    while (aptMainLoop() && running) {
        drawFolderMenu(selected);
        hidScanInput();
        u32 kDown = hidKeysDown();

        if (kDown & KEY_DOWN) selected = (selected + 1) % folderCount;
        if (kDown & KEY_UP) selected = (selected - 1 + folderCount) % folderCount;
        if (kDown & KEY_B) break;

        if (kDown & KEY_A) {
            char url[MAX_URL_LEN] = {0};
            char filename[MAX_FILENAME_LEN] = {0};

            if (showKeyboard(url, sizeof(url), "URLを入力してください") != 0) continue;
            if (showKeyboard(filename, sizeof(filename), "保存ファイル名を入力（拡張子含む）") != 0) continue;

            char fullpath[MAX_PATH_LEN];
            snprintf(fullpath, sizeof(fullpath), "sdmc:%s%s", folders[selected], filename);

            curl_global_init(CURL_GLOBAL_DEFAULT);
            CURL *curl = curl_easy_init();
            if (!curl) {
                printf("curl初期化に失敗しました\n");
                break;
            }

            // サイズチェック
            double filesize = 0;
            curl_easy_setopt(curl, CURLOPT_URL, url);
            curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
            curl_easy_setopt(curl, CURLOPT_HEADER, 1L);
            CURLcode res = curl_easy_perform(curl);
            curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &filesize);

            if (res != CURLE_OK) {
                printf("URL確認に失敗: %s\n", curl_easy_strerror(res));
                curl_easy_cleanup(curl);
                break;
            }

            if (filesize > 17179869184.0) {
                printf("SORRY!!\n16GB超過のためダウンロードできません。\n");
                curl_easy_cleanup(curl);
                break;
            }

            if (filesize > 4294967296.0) {
                if (!confirmProceed("WARNING!!\n 4GB超ファイル。時間がかかる可能性があります。\n続けますか？")) {
                    curl_easy_cleanup(curl);
                    break;
                }
            }

            FILE *fp = fopen(fullpath, "wb");
            if (!fp) {
                printf("ファイルを作成できません: %s\n", fullpath);
                curl_easy_cleanup(curl);
                break;
            }

            // DL実行
            curl_easy_setopt(curl, CURLOPT_NOBODY, 0L);
            curl_easy_setopt(curl, CURLOPT_URL, url);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
            curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);
            res = curl_easy_perform(curl);

            fclose(fp);
            curl_easy_cleanup(curl);

            if (res == CURLE_OK) {
                printf("success OK ダウンロード成功！\n保存先: %s\n", fullpath);
            } else {
                printf("failured... ダウンロード失敗: %s\n", curl_easy_strerror(res));
                remove(fullpath); // 失敗した場合は削除
            }

            printf("\nBで戻る");
            while (aptMainLoop()) {
                hidScanInput();
                if (hidKeysDown() & KEY_B) break;
                gspWaitForVBlank();
            }
        }

        gfxFlushBuffers();
        gfxSwapBuffers();
        gspWaitForVBlank();
    }

    gfxExit();
    return 0;
}
