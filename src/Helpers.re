module Function = {
  let id = x => x;
};

module Option = {
  include Belt.Option;
  type t('a) = option('a);

  let map = (a, f) => map(a, f);
  let flatMap = (a, f) => flatMap(a, f);
  let apply: (t('a), t('a => 'b)) => t('b) =
    (a, m) => flatMap(m, map(a));

  let liftA2: (('a, 'b) => 'c, t('a), t('b)) => t('c) =
    (f, a, b) => apply(b, map(a, f));

  let liftA3 = (f, a, b, c) => apply(c, liftA2(f, a, b));

  let toResult = (a, err) => a->mapWithDefault(Error(err), a => Ok(a));
};

module Result = {
  include Belt.Result;

  let map = (a, f) => map(a, f);
  let flatMap = (a, f) => flatMap(a, f);
  let apply: (t('a, 'e), t('a => 'b, 'e)) => t('ok, 'e) =
    (a, m) => flatMap(m, map(a));

  let liftA2: (('a, 'b) => 'c, t('a, 'e), t('b, 'e)) => t('c, 'e) =
    (f, a, b) => apply(b, map(a, f));
  let liftA3 = (f, a, b, c) => apply(c, liftA2(f, a, b));
  let toOption = a => a->mapWithDefault(None, a => Some(a));
};
