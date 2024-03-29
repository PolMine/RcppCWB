% Generated by roxygen2: do not edit by hand
% Please edit documentation in R/cl.R
\name{corpus_data_dir}
\alias{corpus_data_dir}
\alias{corpus_info_file}
\alias{corpus_full_name}
\alias{corpus_p_attributes}
\alias{corpus_s_attributes}
\alias{corpus_properties}
\alias{corpus_property}
\alias{corpus_registry_dir}
\title{Get information from registry file}
\usage{
corpus_data_dir(corpus, registry = Sys.getenv("CORPUS_REGISTRY"))

corpus_info_file(corpus, registry = Sys.getenv("CORPUS_REGISTRY"))

corpus_full_name(corpus, registry = Sys.getenv("CORPUS_REGISTRY"))

corpus_p_attributes(corpus, registry = Sys.getenv("CORPUS_REGISTRY"))

corpus_s_attributes(corpus, registry = Sys.getenv("CORPUS_REGISTRY"))

corpus_properties(corpus, registry = Sys.getenv("CORPUS_REGISTRY"))

corpus_property(corpus, registry = Sys.getenv("CORPUS_REGISTRY"), property)

corpus_registry_dir(corpus)
}
\arguments{
\item{corpus}{A length-one \code{character} vector with the corpus ID.}

\item{registry}{A length-one \code{character} vector with the registry directory.}

\item{property}{A corpus property defined in the registry file (.}
}
\description{
Extract information from the internal C representation of registry data.
}
\details{
\code{corpus_data_dir()} will return the data directory (class \code{fs_path})
where the binary files of a corpus are kept (a directory also known as
'home' directory).

\code{corpus_info_file()} will return the path to the info file for a
corpus (class \code{fs_path} object). If info file does not exist or INFO line
is missing in the registry file, \code{NA} is returned.

\code{corpus_full_name()} will return the full name of the corpus defined
in the registry file.

\code{corpus_p_attributes()} returns a \code{character} vector with the
positional attributes of a corpus.

\code{corpus_s_attributes()} returns a \code{character} vector with the
structural attributes of a corpus.

\code{corpus_properties()} returns a \code{character} vector with the corpus
properties defined in the registry file. If the corpus cannot be located,
\code{NA} is returned.

\code{corpus_property()} returns the value of a corpus property defined
in the registry file, or \code{NA} if the corpus does not exist, is not loaded
of if the property requested is undefined.

\code{corpus_get_registry()} will extract the registry directory with the
registry file defining a corpus from the internal C representation of
loaded corpora. The \code{character} vector that is returned may be > 1 if there
are several corpora with the same id defined in registry files in different
(registry) directories. If the corpus is not found, \code{NA} is returned.
}
\examples{
corpus_data_dir("REUTERS", registry = get_tmp_registry())
corpus_info_file("REUTERS", registry = get_tmp_registry())
corpus_full_name("REUTERS", registry = get_tmp_registry())
corpus_p_attributes("REUTERS", registry = get_tmp_registry())
corpus_s_attributes("REUTERS", registry = get_tmp_registry())
corpus_properties("REUTERS", registry = get_tmp_registry())
corpus_property(
  "REUTERS",
  registry = get_tmp_registry(),
  property = "language"
)
corpus_registry_dir("REUTERS")
corpus_registry_dir("FOO") # NA returned
}
