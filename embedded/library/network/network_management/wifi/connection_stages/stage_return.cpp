///////////////////////////////////////////////////
///////// StageReturn /////////////////////////////
///////////////////////////////////////////////////

template<typename R>
R StageReturn<R>::get_return_value() {
  return this->return_value;
}

template<typename R>
void StageReturn<R>::set_return_value(R return_value) {
  this->return_value = return_value;
}

template<typename R>
NetworkException StageReturn<R>::get_exception() {
  return this->exception;
}

template<typename R>
void StageReturn<R>::set_exception(NetworkException exception) {
  this->exception = exception;
}