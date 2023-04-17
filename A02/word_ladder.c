//
// AED, November 2022 (Tomás Oliveira e Silva)
//
// Second practical assignement (speed run)
//
// Place your student numbers and names here
//   N.Mec. XXXXXX  Name: XXXXXXX
//
// Do as much as you can
//   1) MANDATORY: complete the hash table code
//      *) hash_table_create
//      *) hash_table_grow
//      *) hash_table_free
//      *) find_word
//      +) add code to get some statistical data about the hash table
//   2) HIGHLY RECOMMENDED: build the graph (including union-find data) -- use the similar_words function...
//      *) find_representative
//      *) add_edge
//   3) RECOMMENDED: implement breadth-first search in the graph
//      *) breadh_first_search
//   4) RECOMMENDED: list all words belonginh to a connected component
//      *) breadh_first_search
//      *) list_connected_component
//   5) RECOMMENDED: find the shortest path between to words
//      *) breadh_first_search
//      *) path_finder
//      *) test the smallest path from bem to mal
//         [ 0] bem
//         [ 1] tem
//         [ 2] teu
//         [ 3] meu
//         [ 4] mau
//         [ 5] mal
//      *) find other interesting word ladders
//   6) OPTIONAL: compute the diameter of a connected component and list the longest word chain
//      *) breadh_first_search
//      *) connected_component_diameter
//   7) OPTIONAL: print some statistics about the graph
//      *) graph_info
//   8) OPTIONAL: test for memory leaks
//

#include <stdio.h>
#include <stdlib.h> 


#define _max_word_size_  32
//
// data structures (SUGGESTION --- you may do it in a different way)
//

typedef struct adjacency_node_s  adjacency_node_t;
typedef struct hash_table_node_s hash_table_node_t;
typedef struct hash_table_s      hash_table_t;

struct adjacency_node_s
{
  adjacency_node_t *next;            // link to th enext adjacency list node
  hash_table_node_t *vertex;         // the other vertex
};

struct hash_table_node_s
{
  // the hash table data
  char word[_max_word_size_];        // the word
  hash_table_node_t *next;           // next hash table linked list node
  // the vertex data
  adjacency_node_t *head;            // head of the linked list of adjancency edges
  int visited;                       // visited status (while not in use, keep it at 0)
  hash_table_node_t *previous;       // breadth-first search parent
  // the union find data
  hash_table_node_t *representative; // the representative of the connected component this vertex belongs to
  int number_of_vertices;            // number of vertices of the conected component (only correct for the representative of each connected component)
  int number_of_edges;               // number of edges of the conected component (only correct for the representative of each connected component)
};

struct hash_table_s
{
  unsigned int hash_table_size;      // the size of the hash table array
  unsigned int number_of_entries;    // the number of entries in the hash table
  unsigned int number_of_edges;      // number of edges (for information purposes only)
  hash_table_node_t **heads;         // the heads of the linked lists
};


//
// allocation and deallocation of linked list nodes (done)
//

static adjacency_node_t *allocate_adjacency_node(void)
{
  adjacency_node_t *node;

  node = (adjacency_node_t *)malloc(sizeof(adjacency_node_t));
  if(node == NULL)
  {
    fprintf(stderr,"allocate_adjacency_node: out of memory\n");
    exit(1);
  }
  return node;
}

static void free_adjacency_node(adjacency_node_t *node)
{
  free(node);
}

static hash_table_node_t *allocate_hash_table_node(void)
{
  hash_table_node_t *node;

  node = (hash_table_node_t *)malloc(sizeof(hash_table_node_t));
  if(node == NULL)
  {
    fprintf(stderr,"allocate_hash_table_node: out of memory\n");
    exit(1);
  }
  return node;
}

static void free_hash_table_node(hash_table_node_t *node)
{
  free(node);
}


//
// hash table stuff (mostly to be done)
//

unsigned int crc32(const char *str)
{
  static unsigned table[256];
  unsigned int crc;

  if(table[1] == 0u) // do we need to initialize the table[] array?
  {
    unsigned int i,j;

    for(i = 0u;i < 256u;i++)
      for(table[i] = i,j = 0u;j < 8u;j++)
        if(table[i] & 1u)
          table[i] = (table[i] >> 1) ^ 0xAED00022u; // "magic" constant
        else
          table[i] >>= 1;
  }
  crc = 0xAED02022u; // initial value (chosen arbitrarily)
  while(*str != '\0')
    crc = (crc >> 8) ^ table[crc & 0xFFu] ^ ((unsigned int)*str++ << 24);
  return crc;
}

static hash_table_t *hash_table_create(void)
{
  hash_table_t *hash_table;
  unsigned int i;

  hash_table = (hash_table_t *)malloc(sizeof(hash_table_t));
  if(hash_table == NULL)
  {
    fprintf(stderr,"create_hash_table: out of memory\n");
    exit(1);
  }
  //
  // complete this
  //
  hash_table->hash_table_size = 250;
  hash_table->heads = (hash_table_node_t **)malloc(sizeof(hash_table_node_t *) * hash_table->hash_table_size);
  hash_table->number_of_entries = 0;
  hash_table->number_of_edges = 0;
  for(i = 0;i < hash_table->hash_table_size;i++)
    hash_table->heads[i] = NULL;
  return hash_table;
}


static void hash_table_grow(hash_table_t *hash_table)
{
  hash_table_node_t **old_heads, **new_heads, *node, *next;
  unsigned int old_size, i;

  // save a pointer to the old array of linked list heads and its size
  old_heads = hash_table->heads;
  old_size = hash_table->hash_table_size;

  // create a new hash table with a larger size
  hash_table->hash_table_size *= 2;
  new_heads = (hash_table_node_t **)malloc(hash_table->hash_table_size * sizeof(hash_table_node_t *));
  // check for out of memory
  for (i = 0u; i < hash_table->hash_table_size; i++)
    new_heads[i] = NULL;

  if (new_heads == NULL)
  {
    fprintf(stderr, "hash_table_grow: out of memory");
    exit(1);
  }

  // run the hash function for old values with new size

  for (i = 0u; i < old_size; i++)
  {
    node = old_heads[i];
    while (node != NULL)
    {

      next = node->next;

      size_t index = crc32(node->word) % hash_table->hash_table_size;
      node->next = new_heads[index];
      new_heads[index] = node;

      node = next;
    }
  }
  free(old_heads);
  hash_table->heads = new_heads;

}

static void hash_table_free(hash_table_t *hash_table)
{
  hash_table_node_t *node;
  hash_table_node_t *temp;
  adjacency_node_t *adj_node;
  adjacency_node_t *temp_adj;
  unsigned int i;

  for (i = 0; i < hash_table->hash_table_size; i++)
  {
    node = hash_table->heads[i];
    while (node != NULL)
    {
      temp = node;
      adj_node = node->head;
      while (adj_node != NULL)
      {
        temp_adj = adj_node;
        adj_node = adj_node->next;
        free(temp_adj);
      }
      node = node->next;
      free(temp);
    }
  }

  free(hash_table->heads);

  free(hash_table);
}


static hash_table_node_t *find_word(hash_table_t *hash_table,const char *word,int insert_if_not_found)
{
  hash_table_node_t *node;
  unsigned int i;

  // printf("word: %s\n", word);
  i = crc32(word) % hash_table->hash_table_size;
  node = hash_table->heads[i];
  while (node != NULL)
  {
    if (strcmp(node->word, word) == 0)
      return node;
    node = node->next;
  }

  if (insert_if_not_found && strlen(word) < _max_word_size_)
  {
    node = allocate_hash_table_node();
    strncpy(node->word, word, _max_word_size_);
    node->representative = node;
    node->next = hash_table->heads[i];
    node->previous = NULL;
    node->number_of_edges = 0;
    node->number_of_vertices = 1;
    node->visited = 0;
    node->head = NULL;
    hash_table->heads[i] = node;
    hash_table->number_of_entries++;
    if (hash_table->number_of_entries > hash_table->hash_table_size)
      hash_table_grow(hash_table);

    return node;
  }
  
  return NULL;
}

static void hash_stats(hash_table_t *hash_table){
  printf("\n\nTamanho da Hash table: %u\n", hash_table->hash_table_size);
  printf("Número de entradas: %u\n", hash_table->number_of_entries);
  printf("Tamanho médio das Heads com Heads vazias: %f\n",(float)hash_table->number_of_entries / hash_table->hash_table_size);

  hash_table_node_t *node;
  unsigned int MaxHead = 0;
  unsigned int MinHead = _max_word_size_;
  unsigned int conta = 0;
  for(unsigned int i = 0u;i < hash_table->hash_table_size;i++){
    unsigned int sizeHeads=0;

    for(node = hash_table->heads[i];node != NULL;node = node->next){
      sizeHeads++;
    }

    if (sizeHeads != 0)
    {
      conta++;
    }


    if( sizeHeads > MaxHead){
      MaxHead = sizeHeads;
    }
    if( sizeHeads < MinHead && sizeHeads != 0){
      MinHead = sizeHeads;
    }

  }
  printf("Tamanho médio das Heads sem Heads vazias: %f\n",(float)hash_table->number_of_entries / conta);
  printf("Tamanho da maior Linked List: %u\n", MaxHead);
  printf("Tamanho da menor Linked List: %u\n\n", MinHead);
}
//
// add edges to the word ladder graph (mostly do be done)
//

static hash_table_node_t *find_representative(hash_table_node_t *node)
{
  hash_table_node_t *representative,*next_node;

  hash_table_node_t *node_atual;
  for (representative = node; representative != representative->representative; representative = representative->representative)
  ;

  for (node_atual = node; node_atual != representative; node_atual = next_node)
  {
    next_node  = node_atual->representative;
    node_atual->representative = representative;
  }

  return representative;

}

static void add_edge(hash_table_t *hash_table, hash_table_node_t *from, const char *word)
{
  hash_table_node_t *to, *from_representative, *to_representative;

  from_representative = find_representative(from);
  to = find_word(hash_table, word, 0);

  if (to == from)
    return;

  to_representative = find_representative(to);

  if (from_representative == to_representative)
  {
    from_representative->number_of_vertices++;
  }else{
    from_representative->representative = to_representative; //mete o representante do from igual ao do to
    to_representative->number_of_vertices += from_representative->number_of_vertices; //soma o numero de vertices do from ao do to
    to_representative->number_of_edges += from_representative->number_of_edges; //soma o numero de arestas do from ao do to
  }

  adjacency_node_t *linkfrom = allocate_adjacency_node(); //aloca memoria para o linkfrom
  adjacency_node_t *linkto = allocate_adjacency_node(); //aloca memoria para o linkto

  linkfrom->vertex = to; //mete o vertice do linkfrom igual ao to
  linkfrom->next = from->head;//mete o next do linkfrom igual ao head do from
  from->head = linkfrom; //mete o head do from igual ao linkfrom

  linkto->vertex = from; //mete o vertice do linkto igual ao from
  linkto->next = to->head; //mete o next do linkto igual ao head do to
  to->head = linkto; //mete o head do to igual ao linkto

  from_representative->number_of_edges++; //incrementa o numero de arestas do from
  to_representative->number_of_edges++; //incrementa o numero de arestas do to
  hash_table->number_of_edges++; //incrementa o numero de arestas da hash table
  
  return;
}

//
// generates a list of similar words and calls the function add_edge for each one (done)
//
// man utf8 for details on the uft8 encoding
//

static void break_utf8_string(const char *word,int *individual_characters)
{
  int byte0,byte1;

  while(*word != '\0')
  {
    byte0 = (int)(*(word++)) & 0xFF;
    if(byte0 < 0x80)
      *(individual_characters++) = byte0; // plain ASCII character
    else
    {
      byte1 = (int)(*(word++)) & 0xFF;
      if((byte0 & 0b11100000) != 0b11000000 || (byte1 & 0b11000000) != 0b10000000)
      {
        fprintf(stderr,"break_utf8_string: unexpected UFT-8 character\n");
        exit(1);
      }
      *(individual_characters++) = ((byte0 & 0b00011111) << 6) | (byte1 & 0b00111111); // utf8 -> unicode
    }
  }
  *individual_characters = 0; // mark the end!
}

static void make_utf8_string(const int *individual_characters,char word[_max_word_size_])
{
  int code;

  while(*individual_characters != 0)
  {
    code = *(individual_characters++);
    if(code < 0x80)
      *(word++) = (char)code;
    else if(code < (1 << 11))
    { // unicode -> utf8
      *(word++) = 0b11000000 | (code >> 6);
      *(word++) = 0b10000000 | (code & 0b00111111);
    }
    else
    {
      fprintf(stderr,"make_utf8_string: unexpected UFT-8 character\n");
      exit(1);
    }
  }
  *word = '\0';  // mark the end
}

static void similar_words(hash_table_t *hash_table,hash_table_node_t *from)
{
  static const int valid_characters[] =
  { // unicode!
    0x2D,                                                                       // -
    0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x4B,0x4C,0x4D,           // A B C D E F G H I J K L M
    0x4E,0x4F,0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5A,           // N O P Q R S T U V W X Y Z
    0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6A,0x6B,0x6C,0x6D,           // a b c d e f g h i j k l m
    0x6E,0x6F,0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7A,           // n o p q r s t u v w x y z
    0xC1,0xC2,0xC9,0xCD,0xD3,0xDA,                                              // Á Â É Í Ó Ú
    0xE0,0xE1,0xE2,0xE3,0xE7,0xE8,0xE9,0xEA,0xED,0xEE,0xF3,0xF4,0xF5,0xFA,0xFC, // à á â ã ç è é ê í î ó ô õ ú ü
    0
  };
  int i,j,k,individual_characters[_max_word_size_];
  char new_word[2 * _max_word_size_];

  break_utf8_string(from->word,individual_characters);
  for(i = 0;individual_characters[i] != 0;i++)
  {
    k = individual_characters[i];
    for(j = 0;valid_characters[j] != 0;j++)
    {
      individual_characters[i] = valid_characters[j];
      make_utf8_string(individual_characters,new_word);
      // avoid duplicate cases
      if(strcmp(new_word,from->word) > 0){

        //+ rapido
        if (find_word(hash_table,new_word,0)!=NULL)
        {
          add_edge(hash_table,from,new_word);
        }

      }

    }
    individual_characters[i] = k;
  }
}

//
// breadth-first search (to be done)
//
// returns the number of vertices visited; if the last one is goal, following the previous links gives the shortest path between goal and origin
//

  
static int breadh_first_search(int maximum_number_of_vertices, hash_table_node_t **list_of_vertices,hash_table_node_t *origin,hash_table_node_t *goal)
{
  int front = 0, back = 0;
  list_of_vertices[back++] = origin;

  origin->visited = 1;
  origin->previous = NULL;

  int finish = 0;

  while (front < back)
  {
    hash_table_node_t *vertex = list_of_vertices[front++];
    if (finish == 1)
    {
      break;
    }

    adjacency_node_t *adjacent = vertex->head;
    while (adjacent != NULL)
    {

      if (adjacent->vertex->visited == 0)
        {
          list_of_vertices[back++] = adjacent->vertex;
          adjacent->vertex->visited = 1;
          adjacent->vertex->previous = vertex;
          if (adjacent->vertex == goal)
          {
            finish = 1;
            break;
          }
        }
      adjacent = adjacent->next;
    }
  }
  for (int i = 0; i < back; i++)
  {
    list_of_vertices[i]->visited = 0;
  }
  return back;
}

//
// list all vertices belonging to a connected component (complete this)
//

static void list_connected_component(hash_table_t *hash_table,const char *word)
{

  hash_table_node_t *node;

  // encontramos o nó que correspode à variavel dada no argumento
  node = find_word(hash_table, word, 0);

  if (node == NULL)
  {
    return;
  }

  // encontramos o resprensentativo dessa palavra 
  hash_table_node_t *representative = find_representative(node);

  int Nr_max = representative->number_of_vertices;
  hash_table_node_t **list_of_vertices = (hash_table_node_t **)malloc(Nr_max * sizeof(hash_table_node_t *));

  int nrPalavras = breadh_first_search(Nr_max, list_of_vertices, node, NULL);

  int count = 1;
  for(int i = 0;i < nrPalavras;i++){
    printf("[%d] %s\n",count,list_of_vertices[i]->word);
    count++;
  }

  free(list_of_vertices);
}

//
// compute the diameter of a connected component (optional)
//

static int largest_diameter;  //variavel para guardar o maior diametro de entre todas as componentes 
static hash_table_node_t **largest_diameter_example; // array que guarda os nós do maior caminho
static int Sum_Diameters, Num_Diameters; // variaveis para calcular a média dos caminhos

static int connected_component_diameter(hash_table_node_t *node)
{
  int Nr_max = node->number_of_vertices;

  hash_table_node_t **list_of_vertices = (hash_table_node_t **)malloc(Nr_max * sizeof(hash_table_node_t *));
  int nrPalavras = breadh_first_search(Nr_max, list_of_vertices, node, NULL);
  int diametro = 0;

  for(int i = 0;i < nrPalavras;i++){ //----- 1 -----//

    hash_table_node_t **list_of_vertices_Dentro = (hash_table_node_t **)malloc(Nr_max * sizeof(hash_table_node_t *));
    //----- 2 -----//
    int dist = breadh_first_search(Nr_max, list_of_vertices_Dentro, list_of_vertices[i], NULL);
    
    hash_table_node_t *path = list_of_vertices_Dentro[dist-1];
    int realDist = 0;
    //----- 3 -----//
    while (path != NULL)
    {
      realDist++;
      path = path->previous;
    }
    //----- 4 -----//
    if (realDist > diametro)
    {
      diametro = realDist;
    }
    //----- 5 -----//
    if (realDist > largest_diameter)
    {
      largest_diameter = realDist;
      hash_table_node_t *palav = list_of_vertices_Dentro[dist-1];
      free(largest_diameter_example);
      largest_diameter_example = (hash_table_node_t **)malloc(largest_diameter * sizeof(hash_table_node_t *));
      int i = 0;
      //----- 6 -----//
      while (palav != NULL)
      {
        largest_diameter_example[i] = palav;
       
        i++;
        palav = palav->previous;
      }
    }
    free(list_of_vertices_Dentro);
    free(path);
  }
  //----- 7 -----//
  Sum_Diameters = Sum_Diameters + diametro-1;
  Num_Diameters++;
  free(list_of_vertices);
  return largest_diameter;
}

static int find_connected_component_representatives(hash_table_t *hash_table, hash_table_node_t **representatives)
{
  int index = 0;

  for (int i = 0; i < hash_table->hash_table_size; i++)
  {
    for (hash_table_node_t *vertex = hash_table->heads[i]; vertex != NULL; vertex = vertex->next)
    {
      hash_table_node_t *representative = find_representative(vertex);

      if (!representative->visited)
      {
        representatives[index++] = representative->word;
        representative->visited = 1;
      }
    }
  }

  // tornamos o estado dos vertices que estavam visitados para 0
  for (int i = 0; i < hash_table->hash_table_size; i++)
  {
    for (hash_table_node_t *vertex = hash_table->heads[i]; vertex != NULL; vertex = vertex->next)
    {
      vertex->visited = 0;
    }
  }

  return index;
}

//
// find the shortest path from a given word to another given word (to be done)
// //


static void path_finder(hash_table_t *hash_table,const char *from_word,const char *to_word)
{
  hash_table_node_t *fromNode = find_word(hash_table, from_word, 0);
  hash_table_node_t *toNode = find_word(hash_table, to_word, 0);

  if(fromNode == NULL || toNode == NULL){
    return;
  }

  hash_table_node_t *fromRep = find_representative(fromNode);
  hash_table_node_t *toRep = find_representative(toNode);

  if(fromRep != toRep){
    return;
  }

  hash_table_node_t **list_of_vertices = malloc(sizeof(hash_table_node_t *) * fromRep->number_of_vertices);


  for (int i = 0; i < fromRep->number_of_vertices; i++)
  {
    list_of_vertices[i]= NULL;
  }

  if (list_of_vertices == NULL)
  {
    exit(1);
  }

  int goal_indice = breadh_first_search(0, list_of_vertices,fromNode, toNode);

  char short_path[goal_indice-1][_max_word_size_];

  hash_table_node_t *p = list_of_vertices[goal_indice-1];

  int count = 0;
  while (p != NULL)
  {
    strcpy(short_path[count], p->word);
    count++;
    p = p->previous;
  }

  int indice2 = 1;
  for (int i = count-1; i >= 0; i--)
  {
    printf("[%d] %s\n", indice2, short_path[i]);
    indice2++;
  }

  free(list_of_vertices);
  // free(fromNode);
  // free(toNode);
  // free(toRep);
  // free(fromRep);
  // free(p);
}

//
// some graph information (optional)
//

static void graph_info(hash_table_t *hash_table)
{
  hash_table_node_t **representatives = malloc(sizeof(hash_table_node_t *) * hash_table->hash_table_size);
  int nrRepresentative = find_connected_component_representatives(hash_table, representatives);
 
  largest_diameter = 0;
  int distMaior;
      
  Sum_Diameters = 0;
  Num_Diameters = 0;
  
  for (int i = 0; i < nrRepresentative; i++)
  {
    distMaior = connected_component_diameter(representatives[i]);
  }

  free(representatives);
}


int main(int argc,char **argv)
{
  char word[100],from[100],to[100];
  hash_table_t *hash_table;
  hash_table_node_t *node;
  unsigned int i;
  int command;
  FILE *fp;

  // initialize hash table
  hash_table = hash_table_create();
  // read words
  fp = fopen((argc < 2) ? "wordlist-big-latest.txt" : argv[1],"rb");
  
  if(fp == NULL)
  {
    fprintf(stderr,"main: unable to open the words file\n");
    exit(1);
  }
  while(fscanf(fp,"%99s",word) == 1)
    (void)find_word(hash_table,word,1);
  fclose(fp);
 
  // find all similar words
  for(i = 0u;i < hash_table->hash_table_size;i++)
    for(node = hash_table->heads[i];node != NULL;node = node->next)
      similar_words(hash_table,node);

  graph_info(hash_table);
  // ask what to do
  for(;;)
  {
    fprintf(stderr,"Your wish is my command:\n");
    fprintf(stderr,"  1 WORD       (list the connected component WORD belongs to)\n");
    fprintf(stderr,"  2 FROM TO    (list the shortest path from FROM to TO)\n");
    fprintf(stderr,"  3            (print words )\n");
    fprintf(stderr,"  4            (statistics hash )\n");
    fprintf(stderr,"  5            (graf info )\n");
    fprintf(stderr,"  6            (terminate)\n");
    fprintf(stderr,"> ");
    if(scanf("%99s",word) != 1)
      break;
    command = atoi(word);
    if(command == 1)
    {

      if(scanf("%99s",word) != 1)
        break;

      list_connected_component(hash_table,word);

    }
    else if(command == 2)
    {
      if(scanf("%99s",from) != 1)
        break;
      if(scanf("%99s",to) != 1)
        break;

      path_finder(hash_table,from,to);
    }
    else if(command == 6)
      break;

    else if (command == 3)
    {

      for(i = 0u;i < hash_table->hash_table_size;i++)
      {
        for(node = hash_table->heads[i];node != NULL;node = node->next)
        {
          printf("índice = %u -> %s\n",i,node->word);
        }
        printf("\n");
      }
    }
    else if (command == 4)
    {
      hash_stats(hash_table);
    }
    else if (command == 5)
    {
      
      printf("Número de vértices: %d\n", hash_table->number_of_entries);
      printf("Número de arestas: %d\n\n", hash_table->number_of_edges);

      hash_table_node_t **representatives = malloc(sizeof(hash_table_node_t *) * hash_table->hash_table_size);
     
      int nrRepresentative = find_connected_component_representatives(hash_table, representatives);
      printf("Número de componentes conexas: %d\n\n", nrRepresentative);


      int size_small_comp = 10;
      int size_large_comp = 0;
      int sum_size_comp = 0;

      for (int i = 0; i < nrRepresentative; i++)
      {
        int Nr_max = representatives[i]->number_of_vertices;
        hash_table_node_t **list_of_vertices = (hash_table_node_t **)malloc(Nr_max * sizeof(hash_table_node_t *));
        
        int nrPalavras = breadh_first_search(Nr_max, list_of_vertices, representatives[i], NULL);

        sum_size_comp = sum_size_comp + nrPalavras;

        if (size_large_comp < nrPalavras)
        {
          size_large_comp = nrPalavras;
        }

        if (size_small_comp > nrPalavras)
        {
          size_small_comp = nrPalavras;
        }

        free(list_of_vertices);
      }
      printf("Tamanho mais pequeno de uma componente: %d\n", size_small_comp-1);
      printf("Tamanho maior de uma componente: %d\n", size_large_comp-1);

      
      

      printf("\nValor médio dos tamanhos das componentes: %f \n\n", (float)sum_size_comp/nrRepresentative);

      printf("\nlargest: %d\n\n", largest_diameter-1);
      printf("Largest Ladder:\n");
      for (int i = 0; i < largest_diameter; i++)
      {
        printf("[%d] %s\n", i+1, largest_diameter_example[i]->word);
      }
      
      printf("\nValor médio dos diametros: %f \n\n", (float)Sum_Diameters / Num_Diameters);

      free(representatives);
    }
  }

  free(largest_diameter_example);
  hash_table_free(hash_table);
  return 0;
  
}