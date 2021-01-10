exception Error(Js.Exn.t);

/* https://docs.aws.amazon.com/AWSJavaScriptSDK/latest/AWS/Response.html */
module Response = (Req: {type t;}, Res: {type t;}) => {
  type t = Res.t;
};

/* https://docs.aws.amazon.com/AWSJavaScriptSDK/latest/AWS/Request.html */
module Request = (Req: {type t;}, Res: {type t;}) => {
  module Response = Response(Req, Res);
  type t = Req.t;

  [@bs.send]
  external on:
    (
      t,
      [@bs.string] [
        | `validate(t => unit)
        | `build(t => unit)
        | `sign(t => unit)
        | `send(Response.t => unit)
        | `retry(Response.t => unit)
        | `extractError(Response.t => unit)
        | `extractData(Response.t => unit)
        | `success(Response.t => unit)
        | `error((Js.Exn.t, Response.t) => unit)
        | `complete(Response.t => unit)
      ]
    ) =>
    t =
    "on";

  [@bs.send]
  external send:
    (
      t,
      ~callback: (Js.Nullable.t(Js.Exn.t), Js.Nullable.t(Response.t)) => unit
                   =?,
      unit
    ) =>
    unit =
    "send";
};

/* https://docs.aws.amazon.com/AWSJavaScriptSDK/latest/AWS/SNS.html */
module SNS = {
  module Req = {
    type t;
  };
  module Res = {
    type t = {
      .
      "MessageId": string,
      "SequenceNumber": string,
    };
  };
  module Request = Request(Req, Res);
  module Response = Response(Req, Res);

  type t;
  type options;

  [@bs.module "aws-sdk"] [@bs.new]
  external make: (~options: options=?, unit) => t = "SNS";

  [@bs.send]
  external publish:
    (
      t,
      'params,
      (Js.Nullable.t(Js.Exn.t), Js.Nullable.t(Response.t)) => unit
    ) =>
    Request.t =
    "publish";

  let publish: (t, Js.t('params)) => Future.t(result(Response.t, exn)) =
    (sns, params) =>
      Future.make(resolve =>
        publish(sns, params, (err, data) =>
          switch (Js.Nullable.toOption(err), Js.Nullable.toOption(data)) {
          | (_, Some(data)) => resolve(Ok(data))
          | (Some(err), _) => resolve(Error(Error(err)))
          | _ =>
            try(Js.Exn.raiseError("Unexpected Error -- " ++ __LOC__)) {
            | Js.Exn.Error(err) => resolve(Error(Error(err)))
            }
          }
        )
      );
};
