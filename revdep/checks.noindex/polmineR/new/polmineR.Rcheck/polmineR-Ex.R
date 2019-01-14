pkgname <- "polmineR"
source(file.path(R.home("share"), "R", "examples-header.R"))
options(warn = 1)
library('polmineR')

base::assign(".oldSearch", base::search(), pos = 'CheckExEnv')
base::assign(".old_wd", base::getwd(), pos = 'CheckExEnv')
cleanEx()
nameEx("Corpus-class")
### * Corpus-class

flush(stderr()); flush(stdout())

### Name: Corpus
### Title: Corpus class.
### Aliases: Corpus
### Keywords: datasets

### ** Examples

use("polmineR")
REUTERS <- Corpus$new("REUTERS")
infofile <- REUTERS$getInfo()
if (interactive()) REUTERS$showInfo()

# use Corpus class to manage counts
REUTERS <- Corpus$new("REUTERS", p_attribute = "word")
REUTERS$stat

# use Corpus class for creating partitions
REUTERS <- Corpus$new("REUTERS", s_attributes = c("id", "places"))
usa <- partition(REUTERS, places = "usa")
sa <- partition(REUTERS, places = "saudi-arabia", regex = TRUE)

reut <- REUTERS$as.partition()



cleanEx()
nameEx("all-cooccurrences-class")
### * all-cooccurrences-class

flush(stderr()); flush(stdout())

### Name: Cooccurrences-class
### Title: Cooccurrences class for corpus/partition.
### Aliases: Cooccurrences-class as_igraph
###   as.simple_triplet_matrix,Cooccurrences-method
###   as_igraph,Cooccurrences-method subset,Cooccurrences-method
###   decode,Cooccurrences-method kwic,Cooccurrences-method
###   as.sparseMatrix,Cooccurrences-method

### ** Examples

X <- Cooccurrences("REUTERS", p_attribute = "word", left = 2L, right = 2L)
m <- as.simple_triplet_matrix(X)
## Not run: 
##D X <- Cooccurrences("REUTERS", p_attribute = "word", left = 5L, right = 5L)
##D decode(X)
##D sm <- as.sparseMatrix(X)
##D stm <- as.simple_triplet_matrix(X)
## End(Not run)



cleanEx()
nameEx("all_cooccurrences")
### * all_cooccurrences

flush(stderr()); flush(stdout())

### Name: Cooccurrences,character-method
### Title: Get all cooccurrences in corpus/partition.
### Aliases: Cooccurrences,character-method Cooccurrences
###   Cooccurrences,partition-method

### ** Examples

## Not run: 
##D # In a first scenario, we get all cooccurrences for the REUTERS corpus,
##D # excluding stopwords
##D 
##D stopwords <- unname(unlist(
##D   noise(
##D     terms("REUTERS", p_attribute = "word"),
##D     stopwordsLanguage = "en"
##D     )
##D   ))
##D r <- Cooccurrences(
##D   .Object = "REUTERS", p_attribute = "word",
##D   left = 5L, right = 5L, stoplist = stopwords
##D )
##D ll(r) # note that the table in the stat slot is augmented in-place
##D decode(r) # in-place modification, again
##D r <- subset(r, ll > 11.83 & ab_count >= 5)
##D data.table::setorderv(r@stat, cols = "ll", order = -1L)
##D head(r, 25)
##D 
##D if (requireNamespace("igraph", quietly = TRUE)){
##D   r@partition <- enrich(r@partition, p_attribute = "word")
##D   g <- as_igraph(r, as.undirected = TRUE)
##D   plot(g)
##D }
##D 
##D # The next scenario is a cross-check that extracting cooccurrences from
##D # from a Cooccurrences-class object with all cooccurrences and the result
##D # for getting cooccurrences for a single object are identical
##D 
##D a <- cooccurrences(r, query = "oil")
##D a <- data.table::as.data.table(a)
##D 
##D b <- cooccurrences("REUTERS", query = "oil", left = 5, right = 5, p_attribute = "word")
##D b <- data.table::as.data.table(b)
##D b <- b[!word %in% stopwords]
##D 
##D all(b[["word"]][1:5] == a[["word"]][1:5]) # needs to be identical!
##D 
##D 
##D stopwords <- unlist(noise(
##D   terms("GERMAPARLMINI", p_attribute = "word"),
##D   stopwordsLanguage = "german"
##D   )
##D )
##D 
##D # We now filter cooccurrences by keeping only the statistically 
##D # significant cooccurrens, identified by comparison with cooccurrences
##D # derived from a reference corpus
##D 
##D plpr_partition <- partition(
##D   "GERMAPARLMINI", date = "2009-11-10", interjection = "speech",
##D   p_attribute = "word"
##D )
##D plpr_cooc <- Cooccurrences(
##D   plpr_partition, p_attribute = "word",
##D   left = 3L, right = 3L,
##D   stoplist = stopwords,
##D   verbose = TRUE
##D )
##D decode(plpr_cooc)
##D ll(plpr_cooc)
##D 
##D merkel <- partition(
##D   "GERMAPARLMINI", speaker = "Merkel", date = "2009-11-10", interjection = "speech",
##D   regex = TRUE,
##D   p_attribute = "word"
##D )
##D merkel_cooc <- Cooccurrences(
##D   merkel, p_attribute = "word",
##D   left = 3L, right = 3L,
##D   stoplist = stopwords, 
##D   verbose = TRUE
##D )
##D decode(merkel_cooc)
##D ll(merkel_cooc)
##D 
##D merkel_min <- subset(
##D   merkel_cooc,
##D   by = subset(features(merkel_cooc, plpr_cooc), rank_ll <= 50)
##D   )
##D   
##D # Esentially the same procedure as in the previous example, but with 
##D # two positional attributes, so that part-of-speech annotation is 
##D # used for additional filtering.
##D    
##D          
##D protocol <- partition(
##D   "GERMAPARLMINI",
##D   date = "2009-11-10",
##D   p_attribute = c("word", "pos"),
##D   interjection = "speech"
##D )
##D protocol_cooc <- Cooccurrences(
##D   protocol,
##D   p_attribute = c("word", "pos"),
##D   left = 3L, right = 3L
##D   )
##D ll(protocol_cooc)
##D decode(protocol_cooc)
##D 
##D merkel <- partition(
##D   "GERMAPARLMINI",
##D   speaker = "Merkel",
##D   date = "2009-11-10",
##D   interjection = "speech",
##D   regex = TRUE,
##D   p_attribute = c("word", "pos")
##D )
##D merkel_cooc <- Cooccurrences(
##D   merkel,
##D   p_attribute = c("word", "pos"),
##D   left = 3L, right = 3L,
##D   verbose = TRUE
##D )
##D ll(merkel_cooc)
##D decode(merkel_cooc)
##D 
##D f <- features(merkel_cooc, protocol_cooc)
##D f <- subset(f, a_pos %in% c("NN", "ADJA"))
##D f <- subset(f, b_pos %in% c("NN", "ADJA"))
##D f <- subset(f, c(rep(TRUE, times = 50), rep(FALSE, times = nrow(f) - 50)))
##D 
##D merkel_min <- subset(merkel_cooc, by = f)
##D 
##D if (requireNamespace("igraph", quietly = TRUE)){
##D   g <- as_igraph(merkel_min, as.undirected = TRUE)
##D   plot(g)
##D }
##D 
## End(Not run)



cleanEx()
nameEx("as.DocumentTermMatrix")
### * as.DocumentTermMatrix

flush(stderr()); flush(stdout())

### Name: as.TermDocumentMatrix
### Title: Generate TermDocumentMatrix / DocumentTermMatrix.
### Aliases: as.TermDocumentMatrix as.DocumentTermMatrix
###   as.TermDocumentMatrix,character-method
###   as.DocumentTermMatrix,character-method
###   as.TermDocumentMatrix,bundle-method
###   as.DocumentTermMatrix,bundle-method
###   as.TermDocumentMatrix,partition_bundle-method
###   as.DocumentTermMatrix,partition_bundle-method
###   as.DocumentTermMatrix,context-method
###   as.TermDocumentMatrix,context-method

### ** Examples

use("polmineR")
 
# do-it-yourself 
p <- partition("GERMAPARLMINI", date = ".*", regex = TRUE)
pB <- partition_bundle(p, s_attribute = "date")
pB <- enrich(pB, p_attribute="word")
tdm <- as.TermDocumentMatrix(pB, col = "count")
   
 # leave the counting to the as.TermDocumentMatrix-method
pB2 <- partition_bundle(p, s_attribute = "date")
tdm <- as.TermDocumentMatrix(pB2, p_attribute = "word", verbose = TRUE)
   
# diretissima
tdm <- as.TermDocumentMatrix("GERMAPARLMINI", p_attribute = "word", s_attribute = "date")



cleanEx()
nameEx("as.VCorpus")
### * as.VCorpus

flush(stderr()); flush(stdout())

### Name: as.VCorpus
### Title: Coerce partition_bundle to VCorpus.
### Aliases: as.VCorpus as.VCorpus,partition_bundle-method as

### ** Examples

use("polmineR")
p <- partition("GERMAPARLMINI", date = "2009-11-10")
pb <- partition_bundle(p, s_attribute = "speaker")
vc <- as.VCorpus(pb) # works only, if tm-package has not yet been loaded
vc <- as(pb, "VCorpus") # will work if tm-package has been loaded, too



cleanEx()
nameEx("as.markdown")
### * as.markdown

flush(stderr()); flush(stdout())

### Name: as.markdown
### Title: Get markdown-formatted full text of a partition.
### Aliases: as.markdown as.markdown,partition-method
###   as.markdown,plpr_partition-method

### ** Examples

use("polmineR")
P <- partition("REUTERS", places = "argentina")
as.markdown(P)
as.markdown(P, meta = c("id", "places"))
if (interactive()) read(P, meta = c("id", "places"))



cleanEx()
nameEx("as.speeches")
### * as.speeches

flush(stderr()); flush(stdout())

### Name: as.speeches
### Title: Split corpus or partition into speeches.
### Aliases: as.speeches

### ** Examples

use("polmineR")
speeches <- as.speeches(
  "GERMAPARLMINI",
  s_attribute_date = "date", s_attribute_name = "speaker"
)
speeches_count <- count(speeches, p_attribute = "word")
tdm <- as.TermDocumentMatrix(speeches_count, col = "count")

bt <- partition("GERMAPARLMINI", date = "2009-10-27")
speeches <- as.speeches(bt, s_attribute_name = "speaker")
summary(speeches)



cleanEx()
nameEx("blapply")
### * blapply

flush(stderr()); flush(stdout())

### Name: blapply
### Title: apply a function over a list or bundle
### Aliases: blapply blapply,list-method blapply,vector-method
###   blapply,bundle-method

### ** Examples

use("polmineR")
bt <- partition("GERMAPARLMINI", date = ".*", regex=TRUE)
speeches <- as.speeches(bt, s_attribute_date = "date", s_attribute_name = "speaker")
foo <- blapply(speeches, function(x, ...) slot(x, "cpos"))



cleanEx()
nameEx("bundle")
### * bundle

flush(stderr()); flush(stdout())

### Name: bundle-class
### Title: Bundle Class
### Aliases: bundle-class bundle name<-,bundle,character-method
###   length,bundle-method names,bundle-method names<-,bundle,vector-method
###   unique,bundle-method +,bundle,bundle-method +,bundle,textstat-method
###   [[,bundle-method [[<-,bundle-method $,bundle-method $<-,bundle-method
###   sample,bundle-method as.bundle,list-method as.bundle,textstat-method
###   as.data.table.bundle as.matrix,bundle-method subset,bundle-method
###   as.list,bundle-method

### ** Examples

parties <- s_attributes("GERMAPARLMINI", "party")
parties <- parties[-which(parties == "NA")]
party_bundle <- partition_bundle("GERMAPARLMINI", s_attribute = "party")
length(party_bundle)
names(party_bundle)
party_bundle <- enrich(party_bundle, p_attribute = "word")
summary(party_bundle)
parties_big <- party_bundle[[c("CDU_CSU", "SPD")]]
summary(parties_big)
p <- partition("GERMAPARLMINI", date = "2009-11-11")
pb <- partition_bundle(p, s_attribute = "party")
names(pb)
pb[["NA"]] <- NULL
names(pb)
pb <- partition_bundle("GERMAPARLMINI", s_attribute = "party")
pb$SPD # access partition names "SPD" in partition_bundle pb
pb <- partition_bundle("GERMAPARLMINI", s_attribute = "party")
pb$"NA" <- NULL # quotation needed if name is "NA"
use("polmineR")
Ps <- partition_bundle(
  "REUTERS", s_attribute = "id",
  values = s_attributes("REUTERS", "id")
)
Cs <- cooccurrences(Ps, query = "oil", cqp = FALSE, verbose = FALSE, progress = TRUE)
dt <- polmineR:::as.data.table.bundle(Cs, col = "ll")
m <- as.matrix(Cs, col = "ll")



cleanEx()
nameEx("chisquare-method")
### * chisquare-method

flush(stderr()); flush(stdout())

### Name: chisquare
### Title: Perform chisquare-text.
### Aliases: chisquare chisquare,features-method chisquare,context-method
###   chisquare,cooccurrences-method
### Keywords: textstatistics

### ** Examples

use("polmineR")
library(data.table)
m <- partition(
  "GERMAPARLMINI", speaker = "Merkel", interjection = "speech",
  regex = TRUE, p_attribute = "word"
)
f <- features(m, "GERMAPARLMINI", included = TRUE)
f_min <- subset(f, count_coi >= 5)
summary(f_min)

## Not run: 
##D 
##D # A sample do-it-yourself calculation for chisquare:
##D 
##D # (a) prepare matrix with observed values
##D o <- matrix(data = rep(NA, 4), ncol = 2) 
##D o[1,1] <- as.data.table(m)[word == "Weg"][["count"]]
##D o[1,2] <- count("GERMAPARLMINI", query = "Weg")[["count"]] - o[1,1]
##D o[2,1] <- size(f)[["coi"]] - o[1,1]
##D o[2,2] <- size(f)[["ref"]] - o[1,2]
##D 
##D 
##D # prepare matrix with expected values, calculate margin sums first
##D 
##D r <- rowSums(o)
##D c <- colSums(o)
##D N <- sum(o)
##D 
##D e <- matrix(data = rep(NA, 4), ncol = 2) 
##D e[1,1] <- r[1] * (c[1] / N)
##D e[1,2] <- r[1] * (c[2] / N)
##D e[2,1] <- r[2] * (c[1] / N)
##D e[2,2] <- r[2] * (c[2] / N)
##D 
##D 
##D # compute chisquare statistic
##D 
##D y <- matrix(rep(NA, 4), ncol = 2)
##D for (i in 1:2) for (j in 1:2) y[i,j] <- (o[i,j] - e[i,j])^2 / e[i,j]
##D chisquare_value <- sum(y)
##D 
##D as(f, "data.table")[word == "Weg"][["chisquare"]]
## End(Not run)



cleanEx()
nameEx("context-method")
### * context-method

flush(stderr()); flush(stdout())

### Name: context
### Title: Analyze context of a node word.
### Aliases: context as.matrix,context_bundle-method
###   context,partition-method context,character-method
###   context,partition_bundle-method context,cooccurrences-method

### ** Examples

use("polmineR")
p <- partition("GERMAPARLMINI", interjection = "speech")
y <- context(p, query = "Integration", p_attribute = "word")
y <- context(p, query = "Integration", p_attribute = "word", positivelist = "Bildung")
y <- context(
  p, query = "Integration", p_attribute = "word",
  positivelist = c("[aA]rbeit.*", "Ausbildung"), regex = TRUE
)



cleanEx()
nameEx("cooccurrences")
### * cooccurrences

flush(stderr()); flush(stdout())

### Name: cooccurrences
### Title: Get cooccurrence statistics.
### Aliases: cooccurrences cooccurrences,character-method
###   cooccurrences,partition-method cooccurrences,context-method
###   cooccurrences,Corpus-method cooccurrences,partition_bundle-method
###   cooccurrences,Cooccurrences-method

### ** Examples

use("polmineR")
merkel <- partition("GERMAPARLMINI", interjection = "speech", speaker = ".*Merkel", regex = TRUE)
merkel <- enrich(merkel, p_attribute = "word")
cooc <- cooccurrences(merkel, query = "Deutschland")

# use subset-method to filter results
a <- cooccurrences("REUTERS", query = "oil")
b <- subset(a, !is.na(ll))
c <- subset(b, !word %in% tm::stopwords("en"))
d <- subset(c, count_coi >= 5)
e <- subset(c, ll >= 10.83)
format(e)

# using pipe operator may be convenient
if (require(magrittr)){
cooccurrences("REUTERS", query = "oil") %>%
  subset(!is.na(ll)) %>%
  subset(!word %in% tm::stopwords("en")) %>%
  subset(count_coi >= 5) %>%
  subset(ll >= 10.83) %>%
  format()
}



cleanEx()
nameEx("corpus-method")
### * corpus-method

flush(stderr()); flush(stdout())

### Name: corpus
### Title: Get corpus/corpora available or used.
### Aliases: corpus corpus,textstat-method corpus,kwic-method
###   corpus,character-method corpus,bundle-method corpus,missing-method

### ** Examples

use("polmineR")
corpus()

p <- partition("REUTERS", places = "kuwait")
corpus(p)

pb <- partition_bundle("REUTERS", s_attribute = "id")
corpus(pb)



cleanEx()
nameEx("corpus_class")
### * corpus_class

flush(stderr()); flush(stdout())

### Name: corpus-class
### Title: S4 class to wrap information on CWB corpora.
### Aliases: corpus-class zoom $,corpus-method ==,corpus,ANY-method
###   !=,corpus,ANY-method %in%,corpus-method zoom,corpus-method

### ** Examples

corp <- corpus("GERMAPARLMINI")
corp2 <- corp$speaker
corp2@key
x <- corpus("GERMAPARLMINI")
x$date == "2009-10-28"
x <- corpus("GERMAPARLMINI")
x$party != "NA"
x <- corpus("GERMAPARLMINI")
x$date %in% c("2009-10-27", "2009-10-28")
x <- corpus("GERMAPARLMINI")
y <- zoom(x, date == "2009-10-28")

x <- partition("GERMAPARLMINI", interjection = "speech")
m <- zoom(x, date == "2009-10-28" & speaker == "Angela Dorothea Merkel")

not_unknown <- zoom(x, party != c("NA", "FDP"))
s_attributes(not_unknown, "party")



cleanEx()
nameEx("count-method")
### * count-method

flush(stderr()); flush(stdout())

### Name: count
### Title: Get counts.
### Aliases: count count-method count,partition-method
###   count,partition_bundle-method count,character-method
###   count,vector-method count,Corpus-method

### ** Examples

use("polmineR")
debates <- partition("GERMAPARLMINI", date = ".*", regex=TRUE)
count(debates, query = "Arbeit") # get frequencies for one token
count(debates, c("Arbeit", "Freizeit", "Zukunft")) # get frequencies for multiple tokens
  
count("GERMAPARLMINI", query = c("Migration", "Integration"), p_attribute = "word")

debates <- partition_bundle(
  "GERMAPARLMINI", s_attribute = "date", values = NULL,
  regex = TRUE, mc = FALSE, verbose = FALSE
)
y <- count(debates, query = "Arbeit", p_attribute = "word")
y <- count(debates, query = c("Arbeit", "Migration", "Zukunft"), p_attribute = "word")
  
count("GERMAPARLMINI", '"Integration.*"', breakdown = TRUE)

P <- partition("GERMAPARLMINI", date = "2009-11-11")
count(P, '"Integration.*"', breakdown = TRUE)



cleanEx()
nameEx("cqp")
### * cqp

flush(stderr()); flush(stdout())

### Name: cqp
### Title: Tools for CQP queries.
### Aliases: cqp is.cqp check_cqp_query as.cqp

### ** Examples

is.cqp("migration") # will return FALSE
is.cqp('"migration"') # will return TRUE
is.cqp('[pos = "ADJA"] "migration"') # will return TRUE

as.cqp("migration")
as.cqp(c("migration", "diversity"))
as.cqp(c("migration", "diversity"), collapse = TRUE)
as.cqp("migration", normalise.case = TRUE)


check_cqp_query('"Integration.*"') # TRUE, the query is ok
check_cqp_query('"Integration.*') # FALSE, closing quotation mark is missing
check_cqp_query("'Integration.*") # FALSE, closing quotation mark is missing
check_cqp_query(c("'Integration.*", '"Integration.*')) # FALSE too



cleanEx()
nameEx("decode")
### * decode

flush(stderr()); flush(stdout())

### Name: decode
### Title: Decode structural attribute, partition or corpus.
### Aliases: decode

### ** Examples

## Not run: 
##D use("polmineR")
##D 
##D # Scenario 1: Decode one or two s-attributes
##D dt <- decode("REUTERS", s_attribute = "id")
##D dt <- decode("REUTERS", s_attribute = c("topics_cat", "places"))
##D 
##D # Scenario 2: Decode entire corpus
##D dt <- decode("REUTERS")
##D 
##D # Scenario 3: Decode partition
##D p <- partition("REUTERS", places = "kuwait", regex = TRUE)
##D dt <- decode(p)
##D 
##D # Scenario 4: Decode partition_bundle
##D pb <- partition_bundle("REUTERS", s_attribute = "id")
##D dts <- lapply(as.list(pb), decode)
##D dts <- lapply(names(dts), function(n) dts[[n]][, speech_id := n])
##D dt <- data.table::rbindlist(dts)
## End(Not run)



cleanEx()
nameEx("dispersion-method")
### * dispersion-method

flush(stderr()); flush(stdout())

### Name: dispersion
### Title: Dispersion of a query or multiple queries
### Aliases: dispersion dispersion,partition-method
###   dispersion,character-method dispersion,hits-method

### ** Examples

use("polmineR")
test <- partition("GERMAPARLMINI", date = ".*", p_attribute = NULL, regex = TRUE)
integration <- dispersion(
  test, query = "Integration",
  p_attribute = "word", s_attribute = "date"
)
integration <- dispersion(test, "Integration", s_attribute = c("date", "party"))
integration <- dispersion(test, '"Integration.*"', s_attribute = "date", cqp = TRUE)



cleanEx()
nameEx("encoding")
### * encoding

flush(stderr()); flush(stdout())

### Name: encoding
### Title: Get and set encoding.
### Aliases: encoding encoding<- encoding,textstat-method
###   encoding,bundle-method encoding,character-method

### ** Examples

# Get encoding of a corpus.
encoding("REUTERS")

# Get encoding of a partition.
r <- partition("REUTERS", places = "kuwait", regex = TRUE)
encoding(r)

# Get encoding of another class inheriting from textstat (count).
cnt <- count("REUTERS", p_attribute = "word")
encoding(cnt)

# Get encoding of objects in a bundle.
pb <- partition_bundle("REUTERS", s_attribute = "id")
encoding(pb)



cleanEx()
nameEx("features")
### * features

flush(stderr()); flush(stdout())

### Name: features
### Title: Get features by comparison.
### Aliases: features features,partition-method features,count-method
###   features,partition_bundle-method features,ngrams-method
###   features,Cooccurrences-method

### ** Examples

use("polmineR")

kauder <- partition(
  "GERMAPARLMINI",
  speaker = "Volker Kauder", interjection = "speech",
  p_attribute = "word"
  )
all <- partition("GERMAPARLMINI", interjection = "speech", p_attribute = "word")

terms_kauder <- features(x = kauder, y = all, included = TRUE)
top100 <- subset(terms_kauder, rank_chisquare <= 100)
head(top100)

# a different way is to compare count objects
kauder_count <- as(kauder, "count")
all_count <- as(all, "count")
terms_kauder <- features(kauder_count, all_count, included = TRUE)
top100 <- subset(terms_kauder, rank_chisquare <= 100)
head(top100)

speakers <- partition_bundle("GERMAPARLMINI", s_attribute = "speaker")
speakers <- enrich(speakers, p_attribute = "word")
speaker_terms <- features(speakers[[1:5]], all, included = TRUE, progress = TRUE)
dtm <- as.DocumentTermMatrix(speaker_terms, col = "chisquare")



cleanEx()
nameEx("get_token_stream-method")
### * get_token_stream-method

flush(stderr()); flush(stdout())

### Name: get_token_stream
### Title: Get Token Stream Based on Corpus Positions.
### Aliases: get_token_stream get_token_stream,numeric-method
###   get_token_stream,matrix-method get_token_stream,character-method
###   get_token_stream,partition-method get_token_stream,regions-method

### ** Examples

get_token_stream(0:9, corpus = "GERMAPARLMINI", p_attribute = "word")
get_token_stream(0:9, corpus = "GERMAPARLMINI", p_attribute = "word", collapse = " ")
fulltext <- get_token_stream("GERMAPARLMINI", p_attribute = "word")



cleanEx()
nameEx("get_type")
### * get_type

flush(stderr()); flush(stdout())

### Name: get_type
### Title: Get corpus/partition type.
### Aliases: get_type get_type,character-method get_type,Corpus-method
###   get_type,partition-method get_type,partition_bundle-method

### ** Examples

use("polmineR")

get_type("GERMAPARLMINI")

p <- partition("GERMAPARLMINI", date = "2009-10-28")
get_type(p)
is(p)

pb <- partition_bundle("GERMAPARLMINI", s_attribute = "date")
get_type(pb)

gp <- Corpus$new("GERMAPARLMINI") 
get_type(gp)

get_type("REUTERS") # returns NULL - no specialized corpus



cleanEx()
nameEx("highlight")
### * highlight

flush(stderr()); flush(stdout())

### Name: highlight
### Title: Highlight tokens in text output.
### Aliases: highlight highlight,character-method highlight,html-method
###   highlight,kwic-method

### ** Examples

use("polmineR")
P <- partition("REUTERS", places = "argentina")
H <- html(P)
Y <- highlight(H, list(lightgreen = "higher"))
if (interactive()) htmltools::html_print(Y)

# highlight matches for a CQP query
H2 <- highlight(
  H,
  highlight = list(yellow = cpos(hits(P, query = '"prod.*"', cqp = TRUE)))
)

# the method can be used in pipe
if (require("magrittr")){
  P %>% html() %>% highlight(list(lightgreen = "1986")) -> H
  P %>% html() %>% highlight(list(lightgreen = c("1986", "higher"))) -> H
  P %>% html() %>% highlight(list(lightgreen = 4020:4023)) -> H
}

# use highlight for kwic output
K <- kwic("REUTERS", query = "barrel")
K2 <- highlight(K, highlight = list(yellow = c("oil", "price")))
if (interactive()) K2

# use character vector for output, not list
K2 <- highlight(
  K,
  highlight = c(
    green = "pric.",
    red = "reduction",
    red = "decrease",
    orange = "dropped"),
    regex = TRUE
)
if (interactive()) K2



cleanEx()
nameEx("html-method")
### * html-method

flush(stderr()); flush(stdout())

### Name: html
### Title: Generate html from object.
### Aliases: html show,html-method html,character-method
###   html,partition-method html,partition_bundle-method html,kwic-method
###   print.html

### ** Examples

use("polmineR")
P <- partition("REUTERS", places = "argentina")
H <- html(P)
if (interactive()) H # show full text in viewer pane

# html-method can be used in a pipe
if (require("magrittr")){
  H <- partition("REUTERS", places = "argentina") %>% html()
  # use html-method to get from concordance to full text
  K <- kwic("REUTERS", query = "barrels")
  H <- html(K, i = 1, s_attribute = "id")
  H <- html(K, i = 2, s_attribute = "id")
  for (i in 1:length(K)) {
    H <- html(K, i = i, s_attribute = "id")
    if (interactive()){
      show(H)
      userinput <- readline("press 'q' to quit or any other key to continue")
      if (userinput == "q") break
    }
  }
}




cleanEx()
nameEx("kwic-class")
### * kwic-class

flush(stderr()); flush(stdout())

### Name: kwic-class
### Title: kwic (S4 class)
### Aliases: kwic-class [,kwic,ANY,ANY,ANY-method [,kwic-method
###   show,kwic-method knit_print,kwic-method as.character,kwic-method
###   subset,kwic-method as.data.frame,kwic-method length,kwic-method
###   sample,kwic-method enrich,kwic-method view,kwic-method

### ** Examples

use("polmineR")
K <- kwic("GERMAPARLMINI", "Integration")
length(K)
K_min <- K[1]
K_min <- K[1:5]
oil <- kwic("REUTERS", query = "oil")
as.character(oil)



cleanEx()
nameEx("kwic")
### * kwic

flush(stderr()); flush(stdout())

### Name: kwic
### Title: KWIC/concordance output.
### Aliases: kwic kwic,context-method kwic,partition-method
###   kwic,character-method

### ** Examples

use("polmineR")
K <- kwic("GERMAPARLMINI", "Integration")
K <- kwic(
  "GERMAPARLMINI",
  "Integration", left = 20, right = 20,
  s_attributes = c("date", "speaker", "party")
)
K <- kwic(
  "GERMAPARLMINI",
  '"Integration" [] "(Menschen|Migrant.*|Personen)"', cqp = TRUE,
  left = 20, right = 20,
  s_attributes = c("date", "speaker", "party")
)

K <- kwic(
  "GERMAPARLMINI",
  '"Sehr" "geehrte"', cqp = TRUE,
  boundary = "date"
)

P <- partition("GERMAPARLMINI", date = "2009-11-10")
K <- kwic(P, query = "Integration")
K <- kwic(P, query = '"Sehr" "geehrte"', cqp = TRUE, boundary = "date")



cleanEx()
nameEx("ll")
### * ll

flush(stderr()); flush(stdout())

### Name: ll
### Title: Compute Log-likelihood Statistics.
### Aliases: ll ll,features-method ll,context-method
###   ll,cooccurrences-method ll,Cooccurrences-method

### ** Examples

# use ll-method explicitly
oil <- cooccurrences("REUTERS", query = "oil", method = NULL)
oil <- ll(oil)
oil_min <- subset(oil, count_coi >= 3)
if (interactive()) View(format(oil_min))
summary(oil)

# use ll-method on 'Cooccurrences'-object
R <- Cooccurrences("REUTERS", left = 5L, right = 5L, p_attribute = "word")
ll(R)
decode(R)
summary(R)

# use log likelihood test for feature extraction
x <- partition(
  "GERMAPARLMINI", speaker = "Merkel",
  interjection = "speech", regex = TRUE,
  p_attribute = "word"
)
f <- features(x, y = "GERMAPARLMINI", included = TRUE, method = "ll")
f <- features(x, y = "GERMAPARLMINI", included = TRUE, method = NULL)
f <- ll(f)
summary(f)

## Not run: 
##D 
##D # A sample do-it-yourself calculation for log-likelihood:
##D # Compute ll-value for query "oil", and "prices"
##D 
##D oil <- context("REUTERS", query = "oil", left = 5, right = 5)
##D 
##D # (a) prepare matrix with observed values
##D o <- matrix(data = rep(NA, 4), ncol = 2) 
##D o[1,1] <- as(oil, "data.table")[word == "prices"][["count_coi"]]
##D o[1,2] <- count("REUTERS", query = "prices")[["count"]] - o[1,1]
##D o[2,1] <- size(oil)[["coi"]] - o[1,1]
##D o[2,2] <- size(oil)[["ref"]] - o[1,2]
##D 
##D 
##D # (b) prepare matrix with expected values, calculate margin sums first
##D r <- rowSums(o)
##D c <- colSums(o)
##D N <- sum(o)
##D 
##D e <- matrix(data = rep(NA, 4), ncol = 2) # matrix with expected values
##D e[1,1] <- r[1] * (c[1] / N)
##D e[1,2] <- r[1] * (c[2] / N)
##D e[2,1] <- r[2] * (c[1] / N)
##D e[2,2] <- r[2] * (c[2] / N)
##D 
##D 
##D # (c) compute log-likelihood value
##D ll_value <- 2 * (
##D   o[1,1] * log(o[1,1] / e[1,1]) +
##D   o[1,2] * log(o[1,2] / e[1,2]) +
##D   o[2,1] * log(o[2,1] / e[2,1]) +
##D   o[2,2] * log(o[2,2] / e[2,2])
##D )
##D 
##D df <- as.data.frame(cooccurrences("REUTERS", query = "oil"))
##D subset(df, word == "prices")[["ll"]]
## End(Not run)



cleanEx()
nameEx("mail-method")
### * mail-method

flush(stderr()); flush(stdout())

### Name: mail
### Title: Send the result of an analysis by Email.
### Aliases: mail mail-method mail,textstat-method mail,data.frame-method
###   mail,kwic-method

### ** Examples

# Get all (global) options for the polmineR package
grep("polmineR", names(options()), value = TRUE)

# Get options that need to be set
getOption("polmineR.email")
getOption("polmineR.smtp_server")
getOption("polmineR.smtp_port")

# Sample options (let us imagine Donald Duck had a mail-account)
options("polmineR.email" = "donald.duck@duckmail.org")
options("polmineR.smtp_port" = "587")
options("polmineR.smtp_server" = "smtp.duckmail.org")

# This is how you send out results when options are set
# (Note: Mail servers that require authentication are not yet supported.)
## Not run: 
##D y <- cooccurrences("REUTERS", query = "oil")
##D mail(y)
##D 
##D k <- kwic("REUTERS", query = "oil")
##D mail(k)
## End(Not run)



cleanEx()
nameEx("ngrams")
### * ngrams

flush(stderr()); flush(stdout())

### Name: ngrams
### Title: Get N-Grams
### Aliases: ngrams ngrams,partition-method ngrams,character-method
###   ngrams,CorpusOrSubcorpus-method ngrams,partition_bundle-method

### ** Examples

use("polmineR")
P <- partition("GERMAPARLMINI", date = "2009-10-27")
ngramObject <- ngrams(P, n = 2, p_attribute = "word", char = NULL)

# a more complex scenario: get most frequent ADJA/NN-combinations
ngramObject <- ngrams(P, n = 2, p_attribute = c("word", "pos"), char = NULL)
ngramObject2 <- subset(
 ngramObject,
 ngramObject[["1_pos"]] == "ADJA"  & ngramObject[["2_pos"]] == "NN"
 )
ngramObject2@stat[, "1_pos" := NULL][, "2_pos" := NULL]
ngramObject3 <- sort(ngramObject2, by = "count")
head(ngramObject3)



cleanEx()
nameEx("p_attributes")
### * p_attributes

flush(stderr()); flush(stdout())

### Name: p_attributes
### Title: Get p-attributes.
### Aliases: p_attributes p_attributes,character-method

### ** Examples

use("polmineR")
p_attributes("GERMAPARLMINI")



cleanEx()
nameEx("partition")
### * partition

flush(stderr()); flush(stdout())

### Name: partition
### Title: Initialize a partition.
### Aliases: partition partition,character-method
###   partition,environment-method partition,partition-method
###   partition,Corpus-method partition,context-method

### ** Examples

use("polmineR")
spd <- partition("GERMAPARLMINI", party = "SPD", interjection = "speech")
kauder <- partition("GERMAPARLMINI", speaker = "Volker Kauder", p_attribute = "word")
merkel <- partition("GERMAPARLMINI", speaker = ".*Merkel", p_attribute = "word", regex = TRUE)
s_attributes(merkel, "date")
s_attributes(merkel, "speaker")
merkel <- partition(
  "GERMAPARLMINI", speaker = "Angela Dorothea Merkel",
  date = "2009-11-10", interjection = "speech", p_attribute = "word"
  )
merkel <- subset(merkel, !word %in% punctuation)
merkel <- subset(merkel, !word %in% tm::stopwords("de"))
   
# a certain defined time segment
days <- seq(
  from = as.Date("2009-10-28"),
  to = as.Date("2009-11-11"),
  by = "1 day"
)
period <- partition("GERMAPARLMINI", date = days)



cleanEx()
nameEx("partition_bundle-method")
### * partition_bundle-method

flush(stderr()); flush(stdout())

### Name: partition_bundle
### Title: Generate bundle of partitions.
### Aliases: partition_bundle partition_bundle,partition-method
###   partition_bundle,character-method partition_bundle,context-method
###   partition_bundle,partition_bundle-method

### ** Examples

use("polmineR")
bt2009 <- partition("GERMAPARLMINI", date = "2009-.*", regex = TRUE)
pb <- partition_bundle(bt2009, s_attribute = "date", progress = TRUE, p_attribute = "word")
dtm <- as.DocumentTermMatrix(pb, col = "count")
summary(pb)
pb <- partition_bundle("GERMAPARLMINI", s_attribute = "date")
# split up objects in partition_bundle by using partition_bundle-method
use("polmineR")
pb <- partition_bundle("GERMAPARLMINI", s_attribute = "date")
pb2 <- partition_bundle(pb, s_attribute = "speaker", progress = FALSE)

summary(pb2)



cleanEx()
nameEx("partition_class")
### * partition_class

flush(stderr()); flush(stdout())

### Name: partition_class
### Title: Partition class and methods.
### Aliases: partition_class partition-class show,partition-method
###   [,partition,ANY,ANY,ANY-method [,partition-method as.partition_bundle
###   as.partition_bundle,partition-method export export,partition-method
###   split summary,partition-method plpr_partition-class
###   press_partition-class p_attributes,partition-method
###   split,partition-method is.partition $,partition-method
###   ==,partition,ANY-method !=,partition,ANY-method %in%,partition-method
###   zoom,partition-method enrich,partition-method
###   as.regions,partition-method

### ** Examples

m <- partition("GERMAPARLMINI", speaker = "Merkel", regex = TRUE)
m2 <- m$speaker
m2@key
m <- partition("GERMAPARLMINI", speaker = "Merkel", regex = TRUE)
m$date == "2009-10-28"
s <- partition("GERMAPARLMINI", interjection = "speech")
s$party != "NA"
s <- partition("GERMAPARLMINI", interjection = "speech")
s$date %in% c("2009-10-27", "2009-10-28")
m <- partition("GERMAPARLMINI", speaker = "Merkel", regex = TRUE)
y <- zoom(m, date == "2009-10-28")

speeches <- partition("GERMAPARLMINI", interjection = "speech")
m <- zoom(speeches, date == "2009-10-28" & speaker == "Angela Dorothea Merkel")

not_unknown <- zoom(speeches, party != c("NA", "FDP"))
s_attributes(not_unknown, "party")



cleanEx()
nameEx("pmi")
### * pmi

flush(stderr()); flush(stdout())

### Name: pmi
### Title: Calculate Pointwise Mutual Information (PMI).
### Aliases: pmi pmi,context-method

### ** Examples

y <- cooccurrences("REUTERS", query = "oil", method = "pmi")
N <- size(y)[["partition"]]
I <- log2((y[["count_coi"]]/N) / ((count(y) / N) * (y[["count_partition"]] / N)))



cleanEx()
nameEx("polmineR-package")
### * polmineR-package

flush(stderr()); flush(stdout())

### Name: polmineR-package
### Title: polmineR-package
### Aliases: polmineR-package polmineR
### Keywords: package

### ** Examples

use("polmineR") # activate demo corpora included in the package

# Core methods applied to corpus

C <- count("REUTERS", query = "oil")
C <- count("REUTERS", query = c("oil", "barrel"))
C <- count("REUTERS", query = '"Saudi" "Arab.*"', breakdown = TRUE, cqp = TRUE)
D <- dispersion("REUTERS", query = "oil", s_attribute = "id")
K <- kwic("REUTERS", query = "oil")
CO <- cooccurrences("REUTERS", query = "oil")


# Core methods applied to partition

kuwait <- partition("REUTERS", places = "kuwait", regex = TRUE)
C <- count(kuwait, query = "oil")
D <- dispersion(kuwait, query = "oil", s_attribute = "id")
K <- kwic(kuwait, query = "oil", meta = "id")
CO <- cooccurrences(kuwait, query = "oil")


# Go back to full text

p <- partition("REUTERS", id = 127)
read(p)
h <- html(p)
h_highlighted <- highlight(h, highlight = list(yellow = "oil"))
h_highlighted


# Generate term document matrix

pb <- partition_bundle("REUTERS", s_attribute = "id")
cnt <- count(pb, p_attribute = "word")
tdm <- as.TermDocumentMatrix(cnt, col = "count")



cleanEx()
nameEx("read-method")
### * read-method

flush(stderr()); flush(stdout())

### Name: read
### Title: Display full text.
### Aliases: read read,partition-method read,partition_bundle-method
###   read,data.table-method read,hits-method read,kwic-method
###   read,regions-method

### ** Examples

use("polmineR")
merkel <- partition("GERMAPARLMINI", date = "2009-11-10", speaker = "Merkel", regex = TRUE)
read(merkel, meta = c("speaker", "date"))
read(
  merkel,
  highlight = list(yellow = c("Deutschland", "Bundesrepublik"), lightgreen = "Regierung"),
  meta = c("speaker", "date")
)

## Not run: 
##D pb <- as.speeches("GERMAPARLMINI", s_attribute_date = "date", s_attribute_name = "speaker")
##D pb <- pb[[ data.table::as.data.table(summary(pb))[size >= 500][["name"]] ]]
##D pb <- pb[[ 1:10 ]]
##D read(pb)
## End(Not run)



cleanEx()
nameEx("regions_class")
### * regions_class

flush(stderr()); flush(stdout())

### Name: regions
### Title: Regions of a CWB corpus.
### Aliases: regions CorpusOrSubcorpus-class regions-class
###   CorpusOrSubcorpus as.regions as.data.table.regions

### ** Examples

use("polmineR")
P <- partition("GERMAPARLMINI", date = "2009-11-12", speaker = "Jens Spahn")
R <- as.regions(P)



cleanEx()
nameEx("registry")
### * registry

flush(stderr()); flush(stdout())

### Name: registry
### Title: Get registry and data directories.
### Aliases: registry data_dir

### ** Examples

registry() # returns temporary registry directory
registry(pkg = "polmineR") # returns registry directory in polmineR-package

data_dir()
data_dir(pkg = "polmineR")



cleanEx()
nameEx("registry_reset")
### * registry_reset

flush(stderr()); flush(stdout())

### Name: registry_reset
### Title: Reset registry directory.
### Aliases: registry_reset

### ** Examples

x <- system.file(package = "polmineR", "extdata", "cwb", "registry")
registry_reset(registryDir = x)



cleanEx()
nameEx("s_attributes-method")
### * s_attributes-method

flush(stderr()); flush(stdout())

### Name: s_attributes
### Title: Get s-attributes.
### Aliases: s_attributes s_attributes,character-method
###   s_attributes,partition-method

### ** Examples

use("polmineR")
  
s_attributes("GERMAPARLMINI")
s_attributes("GERMAPARLMINI", "date") # dates of plenary meetings
  
P <- partition("GERMAPARLMINI", date = "2009-11-10")
s_attributes(P)
s_attributes(P, "speaker") # get names of speakers



cleanEx()
nameEx("size-method")
### * size-method

flush(stderr()); flush(stdout())

### Name: size
### Title: Get Number of Tokens.
### Aliases: size size,character-method size,partition-method
###   size,DocumentTermMatrix-method size,TermDocumentMatrix-method
###   size,features-method

### ** Examples

use("polmineR")
size("GERMAPARLMINI")
size("GERMAPARLMINI", s_attribute = "date")
size("GERMAPARLMINI", s_attribute = c("date", "party"))

P <- partition("GERMAPARLMINI", date = "2009-11-11")
size(P, s_attribute = "speaker")
size(P, s_attribute = "party")
size(P, s_attribute = c("speaker", "party"))



cleanEx()
nameEx("slice")
### * slice

flush(stderr()); flush(stdout())

### Name: slice
### Title: Virtual class clice.
### Aliases: slice slice-class aggregate,slice-method

### ** Examples

P <- new(
  "partition",
  cpos = matrix(data = c(1:10, 20:29), ncol = 2, byrow = TRUE),
  stat = data.table::data.table()
)
P2 <- aggregate(P)
P2@cpos



cleanEx()
nameEx("terms")
### * terms

flush(stderr()); flush(stdout())

### Name: terms
### Title: Get terms in 'partition' or corpus.
### Aliases: terms terms,partition-method terms,character-method

### ** Examples

use("polmineR")
session <- partition("GERMAPARLMINI", date = "2009-10-27")
words <- terms(session, "word")
terms(session, p_attribute = "word", regex = "^Arbeit.*")
terms(session, p_attribute = "word", regex = c("Arbeit.*", ".*arbeit"))

terms("GERMAPARLMINI", p_attribute = "word")
terms("GERMAPARLMINI", p_attribute = "word", regex = "^Arbeit.*")



cleanEx()
nameEx("textstat-class")
### * textstat-class

flush(stderr()); flush(stdout())

### Name: textstat-class
### Title: S4 textstat superclass.
### Aliases: textstat-class as.data.frame,textstat-method
###   show,textstat-method dim,textstat-method colnames,textstat-method
###   rownames,textstat-method names,textstat-method
###   as.DataTables,textstat-method head,textstat-method
###   tail,textstat-method nrow,textstat-method ncol,textstat-method
###   as.data.table,textstat-method round,textstat-method
###   sort,textstat-method [,textstat,ANY,ANY,ANY-method [[,textstat-method
###   name name<- name,textstat-method name,character-method
###   name<-,textstat,character-method as.bundle +,textstat,textstat-method
###   subset,textstat-method as.data.table.textstat
###   p_attributes,textstat-method knit_print,textstat-method
###   format,textstat-method view,textstat-method

### ** Examples

use("polmineR")
P <- partition("GERMAPARLMINI", date = ".*", p_attribute = "word", regex = TRUE)
y <- cooccurrences(P, query = "Arbeit")

# Standard generic methods known from data.frames work for objects inheriting
# from the textstat class

head(y)
tail(y)
nrow(y)
ncol(y)
dim(y)
colnames(y)

# Use brackets for indexing 

## Not run: 
##D y[1:25]
##D y[,c("word", "ll")]
##D y[1:25, "word"]
##D y[1:25][["word"]]
##D y[which(y[["word"]] %in% c("Arbeit", "Sozial"))]
##D y[ y[["word"]] %in% c("Arbeit", "Sozial") ]
## End(Not run)



cleanEx()
nameEx("tooltips")
### * tooltips

flush(stderr()); flush(stdout())

### Name: tooltips
### Title: Add tooltips to text output.
### Aliases: tooltips tooltips,character-method tooltips,html-method
###   tooltips,kwic-method

### ** Examples

use("polmineR")

P <- partition("REUTERS", places = "argentina")
H <- html(P)
Y <- highlight(H, lightgreen = "higher")
T <- tooltips(Y, list(lightgreen = "Further information"))
if (interactive()) T

# Using the tooltips-method in a pipe ...
if (require("magrittr")){
  P %>%
    html() %>%
    highlight(yellow = c("barrels", "oil", "gas")) %>%
    tooltips(list(yellow = "energy"))
}



cleanEx()
nameEx("use")
### * use

flush(stderr()); flush(stdout())

### Name: use
### Title: Add corpora in R data packages to session registry.
### Aliases: use

### ** Examples

use("polmineR")
corpus()



cleanEx()
nameEx("weigh-method")
### * weigh-method

flush(stderr()); flush(stdout())

### Name: weigh
### Title: Apply Weight to Matrix
### Aliases: weigh weigh,TermDocumentMatrix-method
###   weigh,DocumentTermMatrix-method weigh,count-method
###   weigh,count_bundle-method

### ** Examples

## Not run: 
##D library(data.table)
##D if (require("zoo") && require("devtools") && require("magrittr")){
##D 
##D # Source in function 'get_sentiws' from a GitHub gist
##D gist_url <- file.path(
##D   "gist.githubusercontent.com",
##D   "PolMine",
##D   "70eeb095328070c18bd00ee087272adf",
##D   "raw",
##D   "c2eee2f48b11e6d893c19089b444f25b452d2adb",
##D   "sentiws.R"
##D  )
##D   
##D devtools::source_url(sprintf("https://%s", gist_url))
##D SentiWS <- get_sentiws()
##D 
##D # Do the statistical word context analysis
##D use("GermaParl")
##D options("polmineR.left" = 10L)
##D options("polmineR.right" = 10L)
##D df <- context("GERMAPARL", query = "Islam", p_attribute = c("word", "pos")) %>%
##D   partition_bundle(node = FALSE) %>% 
##D   set_names(s_attributes(., s_attribute = "date")) %>%
##D   weigh(with = SentiWS) %>%
##D   summary()
##D 
##D # Aggregate by year
##D df[["year"]] <- as.Date(df[["name"]]) %>% format("%Y-01-01")
##D df_year <- aggregate(df[,c("size", "positive_n", "negative_n")], list(df[["year"]]), sum)
##D colnames(df_year)[1] <- "year"
##D 
##D # Use shares instead of absolute counts 
##D df_year$negative_share <- df_year$negative_n / df_year$size
##D df_year$positive_share <- df_year$positive_n / df_year$size
##D 
##D # Turn it into zoo object, and plot it
##D Z <- zoo(
##D   x = df_year[, c("positive_share", "negative_share")],
##D   order.by = as.Date(df_year[,"year"])
##D )
##D plot(
##D   Z, ylab = "polarity", xlab = "year",
##D   main = "Word context of 'Islam': Share of positive/negative vocabulary",
##D   cex = 0.8,
##D   cex.main = 0.8
##D )
##D 
##D # Note that we can uses the kwic-method to check for the validity of our findings
##D words_positive <- SentiWS[weight > 0][["word"]]
##D words_negative <- SentiWS[weight < 0][["word"]]
##D kwic("GERMAPARL", query = "Islam", positivelist = c(words_positive, words_negative)) %>%
##D   highlight(lightgreen = words_positive, orange = words_negative) %>%
##D   tooltips(setNames(SentiWS[["word"]], SentiWS[["weight"]]))
##D   
##D }
## End(Not run)



### * <FOOTER>
###
cleanEx()
options(digits = 7L)
base::cat("Time elapsed: ", proc.time() - base::get("ptime", pos = 'CheckExEnv'),"\n")
grDevices::dev.off()
###
### Local variables: ***
### mode: outline-minor ***
### outline-regexp: "\\(> \\)?### [*]+" ***
### End: ***
quit('no')
