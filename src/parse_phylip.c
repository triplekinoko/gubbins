/*
 *  Wellcome Trust Sanger Institute
 *  Copyright (C) 2011  Wellcome Trust Sanger Institute
 *  
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *  
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parse_phylip.h"
#include "alignment_file.h"

#include <zlib.h>
#include <sys/types.h>
#include "kseq.h"

KSEQ_INIT(gzFile, gzread)

int num_samples;
int num_snps;
char ** sequences;
char ** phylip_sample_names;
sample_statistics ** statistics_for_samples;


void update_sequence_base(char new_sequence_base, int sequence_index, int base_index)
{
	sequences[sequence_index][base_index] = new_sequence_base;	
}

void get_sequence_for_sample_name(char * sequence_bases, char * sample_name)
{
	int sequence_index;
	sequence_index = find_sequence_index_from_sample_name( sample_name);
	if(sequence_index < 0)
	{
    printf("Couldnt find sequence name %s with index %d\n", sample_name,sequence_index);
	  exit(1);
  }
	strcpy(sequence_bases, sequences[sequence_index]);
}


int does_column_contain_snps(int snp_column, char reference_base)
{
	int i;
	reference_base = convert_reference_to_real_base_in_column( snp_column,  reference_base);
	for(i = 0; i < num_samples; i++)
	{
		if(sequences[i][snp_column] == '\0' || sequences[i][snp_column] == '\n')
		{
			return 0;	
		}
		
		if(sequences[i][snp_column]  != '-' && toupper(sequences[i][snp_column])  != 'N' && sequences[i][snp_column] != reference_base)
		{
			return 1;
		}
	}   
	return 0;
}


char convert_reference_to_real_base_in_column(int snp_column, char reference_base)
{
	int i;
	if(!(reference_base == '-' || toupper(reference_base) == 'N'))
	{
		return reference_base;
	}
	
	for(i = 0; i < num_samples; i++)
	{
		if(sequences[i][snp_column] == '\0' || sequences[i][snp_column] == '\n')
		{
			return reference_base;	
		}
		
		if(sequences[i][snp_column]  != '-' && toupper(sequences[i][snp_column])  != 'N')
		{
			return sequences[i][snp_column];
		}
	}
	return reference_base;
}

int number_of_samples_from_parse_phylip()
{
	return num_samples;
}

sample_statistics ** get_sample_statistics()
{
	return statistics_for_samples;
}


void get_sample_names_from_parse_phylip(char ** sample_names)
{
	int i;
	for(i = 0; i< num_samples; i++)
	{
		sample_names[i] =  (char *) malloc(MAX_SAMPLE_NAME_SIZE*sizeof(char));
		strcpy(sample_names[i], phylip_sample_names[i]);
	}
}
	

void filter_sequence_bases_and_rotate(char * reference_bases, char ** filtered_bases_for_snps, int number_of_filtered_snps)
{
	int i,j,reference_index;
	
	for(j = 0; j < number_of_filtered_snps; j++)
	{
		filtered_bases_for_snps[j] = (char *) malloc((num_samples+1)*sizeof(char));
	}
		
	for(i = 0; i < num_samples; i++)
	{
		int filtered_base_counter = 0;
		
		for(reference_index = 0; reference_index < num_snps; reference_index++)
		{

			if(reference_bases[reference_index] == '\0')
			{
				break;	
			}
			
			if(reference_bases[reference_index] != '*' && sequences[i][reference_index] != '\0' && sequences[i][reference_index] != '\n')
			{
				filtered_bases_for_snps[filtered_base_counter][i] = sequences[i][reference_index];
				filtered_base_counter++;
			}
		}
	}
	for(j = 0; j < number_of_filtered_snps; j++)
	{
		 filtered_bases_for_snps[j][num_samples] = '\0';	
	}

}


void set_number_of_recombinations_for_sample(char * sample_name, int number_of_recombinations)
{
	int sample_index ;
	sample_index = find_sequence_index_from_sample_name( sample_name);
  if( sample_index == -1)
  {
		return;
	}
	((sample_statistics *) statistics_for_samples[sample_index])->number_of_recombinations = number_of_recombinations;
}


void set_number_of_snps_for_sample(char * sample_name, int number_of_snps)
{
	int sample_index ;
	sample_index = find_sequence_index_from_sample_name( sample_name);
  if( sample_index == -1)
  {
		return;
	}
	
	((sample_statistics *) statistics_for_samples[sample_index])->number_of_snps = number_of_snps;
}

void set_number_of_blocks_for_sample(char * sample_name,int num_blocks)
{
	int sample_index ;
	sample_index = find_sequence_index_from_sample_name( sample_name);
  if( sample_index == -1)
  {
		return;
	}
	
	((sample_statistics *) statistics_for_samples[sample_index])->number_of_blocks = num_blocks;
}



void set_genome_length_without_gaps_for_sample(char * sample_name, int genome_length_without_gaps)
{
	int sample_index ;
	sample_index = find_sequence_index_from_sample_name( sample_name);
  if( sample_index == -1)
  {
		return;
	}
	
	((sample_statistics *) statistics_for_samples[sample_index])->genome_length_without_gaps = genome_length_without_gaps;
}

void set_number_of_bases_in_recombinations(char * sample_name, int bases_in_recombinations)
{
	int sample_index ;
	sample_index = find_sequence_index_from_sample_name( sample_name);
  if( sample_index == -1)
  {
		return;
	}
	((sample_statistics *) statistics_for_samples[sample_index])->bases_in_recombinations = bases_in_recombinations;
}




int find_sequence_index_from_sample_name( char * sample_name)
{
	int i;
	
	for(i =0; i< num_samples; i++)
	{
		if(strcmp(sample_name,phylip_sample_names[i]) == 0)	
		{
			return i;
		}
	}
	return -1;
}

void initialise_statistics()
{
	int i=0;
	statistics_for_samples = (sample_statistics **) malloc((num_samples+1)*sizeof(sample_statistics *));
	for(i=0; i< num_samples; i++)
	{
		sample_statistics * sample_statistics_placeholder;
		sample_statistics_placeholder = (sample_statistics *) malloc(sizeof(sample_statistics));
		
		sample_statistics_placeholder->sample_name = (char *) malloc( MAX_SAMPLE_NAME_SIZE*sizeof(char));
		memcpy(sample_statistics_placeholder->sample_name, phylip_sample_names[i], MAX_SAMPLE_NAME_SIZE);
		
		statistics_for_samples[i] = sample_statistics_placeholder;
	}
}

int number_of_snps_in_phylip()
{
	return num_snps;
}

void load_sequences_from_multifasta_file(char filename[])
{
	int i;

	num_snps    = genome_length(filename);
	num_samples = number_of_sequences_in_file(filename);
	
	sequences = (char **) malloc((num_samples+1)*sizeof(char *));
	phylip_sample_names = (char **) malloc((num_samples+1)*sizeof(char *));
	
	for(i = 0; i < num_samples; i++)
	{
		sequences[i] = (char *) malloc((num_snps+1)*sizeof(char));
		phylip_sample_names[i] = (char *) malloc(MAX_SAMPLE_NAME_SIZE*sizeof(char));
	}
	get_sample_names_for_header(filename, phylip_sample_names, num_samples);
	
  int l;
  i = 0;
  int sequence_number = 0;

 	gzFile fp;
 	kseq_t *seq;

 	fp = gzopen(filename, "r");
 	seq = kseq_init(fp);

 	while ((l = kseq_read(seq)) >= 0) 
 	{
     for(i = 0; i< num_snps; i++)
 		{
 			sequences[sequence_number][i] = toupper(((char *) seq->seq.s)[i]);
 			if(sequences[sequence_number][i] == 'N')
 			{
 				sequences[sequence_number][i]  = '-';
 			}
 		}
     sequence_number++;
   }

 	kseq_destroy(seq);
 	gzclose(fp);

	initialise_statistics();
}

