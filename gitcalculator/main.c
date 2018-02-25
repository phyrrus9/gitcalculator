//
//  main.cpp
//  gitcalculator
//
//  Created by Ethan Laur on 4/12/14.
//  Copyright (c) 2014 Ethan Laur. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

//in finding repositories, look for this:
/*
 <h3 class="repo-list-name">
 <a href="/USERNAME/REPO">REPO</a>
 </h3>
 */

char * * double_alloc(int h, int w)
{
	int i;
	char ** ret;
	ret = malloc(h);
	for (i = 0; i < h; i++)
		ret[i] = malloc(w * sizeof(size_t));
	return ret;
}


int count_repositories(char *username)
{
	FILE *f;
	int count = 0, lnum;
	char cmdbuf[64], c;
	char linebuf[4096]; //there should be SOME endlines
	sprintf(cmdbuf, "curl -s https://github.com/%s | grep -n Repositories", username);
	f = popen(cmdbuf, "r"); //run curl on the site to get the repo listing
	while (fscanf(f, "%d: %s", &lnum, linebuf) > 0)
	{
		//curl -s https://github.com/phyrrus9 | head -n 299 | tail -n 1
		if (strstr(linebuf, "title") == NULL) break;
	}
	pclose(f);
	sprintf(cmdbuf, "curl -s https://github.com/%s | head -n %d | tail -n 1", username, lnum + 2);
	f = popen(cmdbuf, "r");
	fscanf(f, "%s", linebuf); sscanf(linebuf, "%d", &count);
	return count;
}

int name_repositories(char *username, char * * list, int num, int page)
{
	int i = 0;
	FILE *f;
	char cmdbuf[1024], linebuf[4096], *tempbuf, *ptr, c;
	sprintf(cmdbuf, "curl -s https://github.com/%s?page=%d\\&tab=repositories | grep /%s/", username, page+1, username);
	f = popen(cmdbuf, "r"); //run curl on the site to get the repo listing
	while (fscanf(f, "%[^\n]%c", linebuf, &c) > 0 && i < num)
		if (strstr(linebuf, "codeRepository") != NULL) // is a repo line
		{
			ptr = strstr(linebuf, "href=") + 8 + strlen(username);
			*strstr(ptr, "\"") = 0;
			sprintf(list[i++], "%s/%s", username, ptr);
		}
	pclose(f);
	return i;
}

int count_lines(char *username, char *base, int *charout)
{
	char fbuf[512];
	int lines;
	FILE *f;
	sprintf(fbuf, "find %s/%s -type f -print0 | xargs -0 cat | wc -lc", base, username);
	f = popen(fbuf, "r");
	sleep(10);
	fscanf(f, "%d", &lines);
	if (charout != NULL)
		fscanf(f, "%d", charout);
	pclose(f);
	return lines;
}

int clone_repositories(char * * list, int num, char *base, int offset)
{
	int i;
	char clonebuf[256];
	for (i = 0; i < num; i++)
	{
		printf("\r                                                       \r");
		printf("--> %-3d\t : %s\n...   ", i + 1 + offset, list[i]); fflush(stdout);
		fflush(stdout);
		sprintf(clonebuf, "git clone -q https://github.com/%s %s/%s", list[i], base, list[i]);
		system(clonebuf);
	}
	return i;
}

int main(int argc, char * * argv)
{
	char * * repositories, *username = "torvalds", *base = "/tmp/git2", cleancmd[64], * *resultsbuf;
	int i, count, j, lines, chars, reponum, named, page;
	resultsbuf = malloc(sizeof(char *) * argc);
	for (j = 1, reponum = 0; j < argc; j++)
	{
		named = page = lines = chars = 0;
		username = argv[j];
		count = count_repositories(username);
		sprintf(cleancmd, "rm -rf %s/%s", base, username);
		system(cleancmd);
		repositories = malloc(count * 9); //some pad
		for (i = 0; i < count; i++)
			repositories[i] = malloc(64);
		do { named += name_repositories(username, &repositories[named], count, page++); } while (named < count);
		reponum += clone_repositories(repositories, count, base, reponum);
		printf("\rCalculating -> %s", username); fflush(stdout);
		lines = count_lines(username, base, &chars);
		system(cleancmd);
		resultsbuf[j - 1] = malloc(256);
		sprintf((char *)resultsbuf[j - 1], "\r                                     \r"
						   "%-10s: Lines: %-9d\tCharacters: %-12d\n", username, lines, chars);
	}
	for (j = 1; j < argc; ++j)
	{
		printf(resultsbuf[j - 1]);
		free(resultsbuf[j - 1]);
	}
	free(resultsbuf);
	return 0;
}

