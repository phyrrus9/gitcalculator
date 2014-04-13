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
 <h3 class="repolist-name">
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
	int count = 0;
	char cmdbuf[64];
	char linebuf[4096]; //there should be SOME endlines
	sprintf(cmdbuf, "curl -s https://github.com/%s?tab=repositories", username);
	f = popen(cmdbuf, "r"); //run curl on the site to get the repo listing
	while (fscanf(f, "%[^\n]\n", linebuf) != EOF)
		if (strstr(linebuf, "<h3 class=\"repolist-name\">") != NULL)
			++count;
	pclose(f);
	return count;
}

void name_repositories(char *username, char * * list, int num)
{
	int i = 0;
	FILE *f;
	char cmdbuf[1024], linebuf[4096], *tempbuf, *ptr;
	sprintf(cmdbuf, "curl -s https://github.com/%s?tab=repositories", username);
	f = popen(cmdbuf, "r"); //run curl on the site to get the repo listing
	while (fscanf(f, "%[^\n]\n", linebuf) != EOF && i < num)
		if (strstr(linebuf, "<h3 class=\"repolist-name\">") != NULL)
			if (fscanf(f, "%[^\n]\n", linebuf) != EOF)
				if ((ptr = strstr(linebuf, "\">")) != NULL)
					if ((ptr += 2) || 1)
						if ((tempbuf = strstr(ptr, "</a>")) || 1)
							if ((*tempbuf = 0) || 1)
								sprintf(list[i++], "%s/%s", username, ptr);
	pclose(f);
}

void clone_repositories(char * * list, int num, char *base)
{
	int i;
	char clonebuf[256];
	printf("Cloning %d repositories, this can take a while..be patient\n", num);
	for (i = 0; i < num; i++)
	{
		printf("-->%-3d: %s\n", i + 1, list[i]);
		fflush(stdout);
		sprintf(clonebuf, "git clone -q https://github.com/%s %s/%s", list[i], base, list[i]);
		system(clonebuf);
	}
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

int main(int argc, char * * argv)
{
	char * * repositories, *username = "torvalds", *base = "/tmp/git2", cleancmd[64];
	int i, count, j, lines, chars;
	for (j = 1; j < argc; j++)
	{
		username = argv[j];
		count = count_repositories(username);
		sprintf(cleancmd, "rm -rf %s/%s", base, username);
		system(cleancmd);
		repositories = malloc(count * 9); //some pad
		for (i = 0; i < count; i++)
			repositories[i] = malloc(64);
		name_repositories(username, repositories, count);
		clone_repositories(repositories, count, base);
		lines = count_lines(username, base, &chars);
		system(cleancmd);
		printf("%-10s: Lines: %-9d\tCharacters: %-12d\n", username, lines, chars);
	}
}

